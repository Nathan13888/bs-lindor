import torch 

import os
from tqdm import tqdm
import numpy as np
from bs_gym.gymbattlesnake import BattlesnakeEnv

# TODO: execute in parallel
def check_performance(current_policy, opponent, n_opponents=3, n_envs=1000, steps=1500, device=torch.device('cpu')):
    test_env = BattlesnakeEnv(n_threads=os.cpu_count(), n_envs=n_envs, opponents=[opponent for _ in range(n_opponents)], device=device)
    obs = test_env.reset()
    wins = 0
    losses = 0
    completed = np.zeros(n_envs)
    count = 0
    lengths = []

    with torch.no_grad():
        print("Running performance check")
        for step in tqdm(range(steps)):
            if count == n_envs:
                print("Check Performance done @ step", step)
                break
            inp = torch.tensor(obs, dtype=torch.float32).to(device)
            action, _ = current_policy.predict(inp, deterministic=True)
            obs, reward, done, info = test_env.step(action.cpu().numpy().flatten())
            for i in range(test_env.n_envs):
                if completed[i] == 1:
                    continue # Only count each environment once
                if 'episode' in info[i]:
                    if info[i]['episode']['r'] == 1:
                        completed[i] = 1
                        count += 1
                        wins += 1
                        lengths.append(info[i]['episode']['l'])
                    elif info[i]['episode']['r'] == -1:
                        completed[i] = 1
                        losses += 1
                        count += 1
                        lengths.append(info[i]['episode']['l'])

    winrate = wins / n_envs
    print("Wins", wins)
    print("Losses", losses)
    print("Average episode length:", np.mean(lengths))
    
    return winrate