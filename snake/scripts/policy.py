import torch
import torch.nn as nn
import torch.nn.functional as F
from a2c_ppo_acktr.model import Policy, NNBase

# NOTE: CONFIG
device = torch.device('cuda')

# SOURCE: https://github.com/fastai/fastai2/blob/master/fastai2/vision/models/xresnet.py
# kaiming initialization for feature layers
def init_cnn(m):
    if getattr(m, 'bias', None) is not None: nn.init.constant_(m.bias, 0)
    if isinstance(m, (nn.Conv2d,nn.Linear)): nn.init.kaiming_normal_(m.weight)
    for l in m.children(): init_cnn(l)

# DOCS: https://github.com/ikostrikov/pytorch-a2c-ppo-acktr-gail/blob/master/a2c_ppo_acktr/model.py
class SnakePolicyBase(NNBase):
    def __init__(self, num_inputs, recurrent=False, hidden_size=None):
        # hidden_size == output_size of self.policy_head
        if hidden_size is None:
            hidden_size = 512
        super().__init__(recurrent, hidden_size, hidden_size)
        
        self.base = nn.Sequential(
            nn.Conv2d(17, 128, 3),
            nn.ReLU(),
            nn.Conv2d(128, 128, 3),
            nn.ReLU(),
            nn.Conv2d(128, 128, 3),
            nn.ReLU(),
            nn.Conv2d(128, 128, 3),
            nn.ReLU(),
        )
        self.pooling = nn.AdaptiveMaxPool2d(2)
        self.fc1 = nn.Linear(in_features=512, out_features=512)
        self.value_head = nn.Linear(in_features=512, out_features=1)
        self.policy_head = nn.Linear(in_features=512, out_features=512)


        init_cnn(self)
        
    def forward(self, obs, rnn_hxs, masks):
        out = F.relu(self.base(obs))
        out = self.pooling(out).view(-1, 512)
        out = F.relu(self.fc1(out))
        
        value_out = self.value_head(out)
        policy_out = self.policy_head(out)
        
        return value_out, policy_out, rnn_hxs
    

    
class PredictionPolicy(Policy):
    # called by gym
    def predict(self, inputs, deterministic=False):
        # numpy array converted to a tensor
        inputs.clone().detach()
        value, actor_features, rnn_hxs = self.base(inputs, None, None)
        dist = self.dist(actor_features)

        if deterministic:
            action = dist.mode()
        else:
            action = dist.sample()

        return action, value
        
def create_policy(obs_space, act_space, base):
    return PredictionPolicy(obs_space, act_space, base=base)
