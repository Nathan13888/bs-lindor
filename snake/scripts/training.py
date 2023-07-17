import numpy as np
import time
from datetime import datetime
import torch
import torch.nn as nn
import torch.nn.functional as F
from tqdm import tqdm
import matplotlib.pyplot as plt

from a2c_ppo_acktr.algo import PPO
from a2c_ppo_acktr.model import Policy, NNBase
from a2c_ppo_acktr.storage import RolloutStorage
from gym_battlesnake.gymbattlesnake import BattlesnakeEnv
from performance import check_performance

device = torch.device('cuda')

# From https://github.com/fastai/fastai2/blob/master/fastai2/vision/models/xresnet.py
def init_cnn(m):
    if getattr(m, 'bias', None) is not None: nn.init.constant_(m.bias, 0)
    if isinstance(m, (nn.Conv2d,nn.Linear)): nn.init.kaiming_normal_(m.weight)
    for l in m.children(): init_cnn(l)

# See: https://github.com/ikostrikov/pytorch-a2c-ppo-acktr-gail/blob/master/a2c_ppo_acktr/model.py
# for examples
        
class SnakePolicyBase(NNBase):
    ''' Neural Network Policy for our snake. This is the brain '''
    # hidden_size must equal the output size of the policy_head
    def __init__(self, num_inputs, recurrent=False, hidden_size=128): 
        super().__init__(recurrent, hidden_size, hidden_size)
        
        # We'll define a 3-stack CNN with leaky_relu activations and a batchnorm
        # here.
        self.base = nn.Sequential(
            nn.Conv2d(17, 32, 3),
            nn.LeakyReLU(),
            nn.Conv2d(32, 32, 3),
            nn.LeakyReLU(),
            nn.Conv2d(32, 32, 3),
            nn.LeakyReLU(),
        )
        
        # Try yourself: Try different pooling methods
        # We add a pooling layer since it massively speeds up training
        # and reduces the number of parameters to learn.
        self.pooling = nn.AdaptiveMaxPool2d(2)
        
        # Try yourself: Change the number of features
        # 64 channels * 4x4 pooling outputs = 1024
        self.fc1 = nn.Linear(in_features=32*2*2, out_features=128)
        
        # Value head predicts how good the current board is
        self.value_head = nn.Linear(in_features=128, out_features=1)
        
        # Policy network gives action probabilities
        # The output of this is fed into a fully connected layer with 4 outputs
        # (1 for each possible direction)
        self.policy_head = nn.Linear(in_features=128, out_features=128)
        
        # Use kaiming initialization in our feature layers
        init_cnn(self)
        
    def forward(self, obs, rnn_hxs, masks):
        out = F.leaky_relu(self.base(obs))
        out = self.pooling(out).view(-1, 128)
        out = F.leaky_relu(self.fc1(out))
        
        value_out = self.value_head(out)
        policy_out = self.policy_head(out)
        
        return value_out, policy_out, rnn_hxs
    
class PredictionPolicy(Policy):
    """ Simple class that wraps the packaged policy with the prediction method needed by the gym """

    def predict(self, inputs, deterministic=False):
        # Since this called from our gym environment
        # (and passed as a numpy array), we need to convert it to a tensor
        inputs = torch.tensor(inputs, dtype=torch.float).to(device)
        value, actor_features, rnn_hxs = self.base(inputs, None, None)
        dist = self.dist(actor_features)

        if deterministic:
            action = dist.mode()
        else:
            action = dist.sample()

        return action, value
        
def create_policy(obs_space, act_space, base):
    """ Returns a wrapped policy for use in the gym """
    return PredictionPolicy(obs_space, act_space, base=base)

def count_parameters(model):
    return sum(p.numel() for p in model.parameters() if p.requires_grad)


#====

print("Starting environment.")

# Number of parallel environments to generate games in
n_envs = 50
# Number of steps per environment to simulate
n_steps = 400

# The total training set size per iteration is n_envs * n_steps

# The gym environment
env = BattlesnakeEnv(n_threads=1, n_envs=n_envs)

# Storage for rollouts (game turns played and the rewards)
rollouts = RolloutStorage(n_steps,
                          n_envs,
                          env.observation_space.shape,
                          env.action_space,
                          n_steps)
env.close()

# Create our policy as defined above
policy = create_policy(env.observation_space.shape, env.action_space, SnakePolicyBase)
best_old_policy = create_policy(env.observation_space.shape, env.action_space, SnakePolicyBase)

# Lets make the old policy the same as the current one
best_old_policy.load_state_dict(policy.state_dict())

# OR LOAD A SAVED MODEL
# ie: policy = torch.load('saved_models/my_model.pt')

agent = PPO(policy,
            value_loss_coef=0.5,
            entropy_coef=0.01,
            max_grad_norm=0.5,
            clip_param=0.2,
            ppo_epoch=4,
            num_mini_batch=32,
            eps=1e-5,
            lr=1e-3)

print(f"Trainable Parameters: {count_parameters(policy)}")



#====


def get_modelpath(fmt):
    t = datetime.now().strftime('%H_%M_%d_%m_%Y')
    return 'models/'+(fmt.replace("TIME", t)) + '.pt'

print("Starting training.")

# We'll play 4-way matches (TODO: env)
env = BattlesnakeEnv(n_threads=2, n_envs=n_envs, opponents=[policy for _ in range(3)], device=device)
obs = env.reset()
rollouts.obs[0].copy_(torch.tensor(obs))

# How many iterations do we want to run
num_updates = 50

# Send our network and storage to the gpu
policy.to(device)
best_old_policy.to(device)
rollouts.to(device)

# Record mean values to plot at the end
rewards = []
value_losses = []
lengths = []

start = time.time()
for j in range(num_updates):
    episode_rewards = []
    episode_lengths = []
    # Set
    policy.eval()
    print(f"Iteration {j+1}: Generating rollouts")
    for step in tqdm(range(n_steps)):
        with torch.no_grad():
            value, action, action_log_prob, recurrent_hidden_states = policy.act(rollouts.obs[step],
                                                            rollouts.recurrent_hidden_states[step],
                                                            rollouts.masks[step])
        obs, reward, done, infos = env.step(action.cpu().squeeze())
        obs = torch.tensor(obs)
        reward = torch.tensor(reward).unsqueeze(1)

        for info in infos:
            if 'episode' in info.keys():
                episode_rewards.append(info['episode']['r'])
                episode_lengths.append(info['episode']['l'])

        masks = torch.FloatTensor([[0.0] if done_ else [1.0] for done_ in done])
        bad_masks = torch.FloatTensor([[0.0] if 'bad_transition' in info.keys() else [1.0] for info in infos])
        rollouts.insert(obs, recurrent_hidden_states, action, action_log_prob, value, reward, masks, bad_masks)

    with torch.no_grad():
        next_value = policy.get_value(
            rollouts.obs[-1],
            rollouts.recurrent_hidden_states[-1],
            rollouts.masks[-1]
        ).detach()
        
    # Set the policy to be in training mode (switches modules to training mode for things like batchnorm layers)
    policy.train()

    print("Training policy on rollouts...")
    # We're using a gamma = 0.99 and lambda = 0.95
    rollouts.compute_returns(next_value, True, 0.99, 0.95, False)
    value_loss, action_loss, dist_entropy = agent.update(rollouts)
    rollouts.after_update()

    # Set the policy into eval mode (for batchnorms, etc)
    policy.eval()
    
    total_num_steps = (j + 1) * n_envs * n_steps
    end = time.time()
    
    lengths.append(np.mean(episode_lengths))
    rewards.append(np.mean(episode_rewards))
    value_losses.append(value_loss)
    
    # Every 5 iterations, we'll print out the episode metrics
    if (j+1) % 5 == 0:
        print("\n")
        print("=" * 80)
        print("Iteration", j+1, "Results")
        # Check the performance of the current policy against the prior best
        winrate = check_performance(policy, best_old_policy, device=torch.device(device))
        print(f"Winrate vs prior best: {winrate*100:.2f}%")
        print(f"Median Length: {np.median(episode_lengths)}")
        print(f"Max Length: {np.max(episode_lengths)}")
        print(f"Min Length: {np.min(episode_lengths)}")

        # If our policy wins more than 30% of the games against the prior
        # best opponent, update the prior best.
        # Expected outcome for equal strength players is 25% winrate in a 4 player
        # match.
        if winrate > 0.3:
            print("Policy winrate is > 30%. Updating prior best model")
            best_old_policy.load_state_dict(policy.state_dict())
            print("Saving latest best model.")
            torch.save(best_old_policy.state_dict(), get_modelpath(f'weights_TIME_iter{j+1}'))
        else:
            print("Policy has not learned enough yet... keep training!")
        print("-" * 80)

print("Saving final model.")
torch.save(policy.state_dict(), get_modelpath('final_TIME'))

# ===

print("Plotting graphs.")


plt.clf()
plt.title("Average episode length")
plt.ylabel("Length")
plt.xlabel("Iteration")
plt.plot(lengths)
plt.show()

plt.title("Average episode reward")
plt.ylabel("Reward")
plt.xlabel("Iteration")
plt.plot(rewards)
plt.show()