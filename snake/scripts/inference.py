import sys
import os
import logging

import torch
import numpy as np
from bs_gym.gymbattlesnake import BattlesnakeEnv
from a2c_ppo_acktr.storage import RolloutStorage

from policy import SnakePolicyBase, create_policy
from utils import PathHelper
# from utils import device

# TODO: CONFIG FILE
n_envs = 1
n_steps = 600

# torch.backends.cuda.matmul.allow_tf32 = False # Do matmul at TF32 mode.
CPU_THREADS = os.cpu_count()
# device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
# device = torch.device('cuda')
device = torch.device('cpu')
MODEL_GROUP = 'test10'

NUM_LAYERS = 17
LAYER_WIDTH = 23
LAYER_HEIGHT = 23

# configure logger
logger = logging.getLogger('inference')

logger.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s | %(levelname)s | %(message)s')
stdout_handler = logging.StreamHandler(sys.stdout)
stdout_handler.setLevel(logging.DEBUG)
stdout_handler.setFormatter(formatter)
logger.addHandler(stdout_handler)


# get latest model
MODEL_PATH = None
u = PathHelper()
u.set_modelgroup(MODEL_GROUP, read_tmp=True)
MODEL_PATH, _ = u.get_latest_model()

print('Loading model from:', MODEL_PATH)

if MODEL_PATH is None:
    sys.exit(1)

# configure rollouts
tmp_env = BattlesnakeEnv(n_threads=CPU_THREADS, n_envs=n_envs)
# rollouts = RolloutStorage(n_steps,
#                           n_envs,
#                           tmp_env.observation_space.shape,
#                           tmp_env.action_space,
#                           n_steps)
tmp_env.close()

# Load policy
policy = create_policy(tmp_env.observation_space.shape, tmp_env.action_space, SnakePolicyBase)
policy.load_state_dict(torch.load(MODEL_PATH))

policy.to(device)
policy.eval()


# environment
opponent_policies = [policy for _ in range(7)]

# TODO: ???
# Do some sanity checks


#     def predict(self):
#         obs = env.reset()
#         with torch.no_grad():
#             inp = torch.tensor(obs, dtype=torch.float32).to(device)
#             action, _ = current_policy.predict(inp, deterministic=True)
#             obs, reward, done, info = env.step(action.cpu().numpy().flatten())

#             print(reward)
#             print(done)
#             print(info)
#             print(obs)



# Start server to accept inference requests
from fastapi import FastAPI
from typing import List
from pydantic import BaseModel

import time
from collections import defaultdict

# dictionary of strings to BattlesnakeEnv
environments = defaultdict(lambda: None)

# def get_env(id) -> BattlesnakeEnv:
#     global environments, opponent_policies

#     # check if environments contains id
#     if id not in environments:
#         # create a new environment
#         env = BattlesnakeEnv(n_threads=CPU_THREADS, n_envs=n_envs, opponents=opponent_policies, device=device)
#         environments[id] = env

#     # return from dictionary
#     return environments[id]

app = FastAPI()

class InferenceRequest(BaseModel):
    id: str
    width: int
    height: int
    input: 'Frames'

class Frames(BaseModel):
    l0_health: List[List[int]]
    l1_bodies: List[List[int]]
    l2_segments: List[List[int]]
    l3_snake_length: List[List[int]]
    l4_food: List[List[int]]
    l5_board: List[List[int]]
    l6_head_mask: List[List[int]]
    l7_tail_mask: List[List[int]]
    l8_bodies_gte: List[List[int]]
    l9_bodies_lt: List[List[int]]
    alive_count: List[List[List[int]]] # 7 layers

InferenceRequest.model_rebuild()

# @app.post("/api/start")
# def start():
#     # TODO: logging
#     get_env(id)

# @app.post("/api/end")
# def stop():
#     global environments

#     # delete environment from dictionary
#     if id in environments:
#         environments[id].close()
#         del environments[id]
#         logger.info(f"Removed environment with id {id}")
#     else:
#         logger.warning(f"Environment with id {id} does not exist")

@app.post("/api/predict")
def predict(req: InferenceRequest):
    # TODO: logging with UUID
    # TODO: time inference

    id = req.id
    # width = req.width
    # height = req.height
    frames = req.input

    # create observation
    # obs = np.zeros(shape=(n_envs, NUM_LAYERS, LAYER_WIDTH, LAYER_HEIGHT), dtype=np.uint8)
    array = [
        frames.l0_health,
        frames.l1_bodies,
        frames.l2_segments,
        frames.l3_snake_length,
        frames.l4_food,
        frames.l5_board,
        frames.l6_head_mask,
        frames.l7_tail_mask,
        frames.l8_bodies_gte,
        frames.l9_bodies_lt,
        *frames.alive_count,
    ]
    obs = np.asarray(array, dtype=np.uint8)

    # logger.info(f'Received inference for {id}')

    startTime = time.time()

    # get environment
    # env = get_env(id)


    # execute interence on environment
    with torch.no_grad():


        inp = torch.tensor(obs, dtype=torch.float32).to(device)

        action, value = policy.predict(inp, deterministic=True)

        # test_env = BattlesnakeEnv(n_threads=os.cpu_count(), n_envs=1, opponents=[policy for _ in range(1)], device=device)
        # obs, reward, done, info = test_env.step(action.cpu().numpy().flatten())


        # tmp = ''
        # tmp += 25 * '-' + '\n'
        # for x in range(23):
        #     tmp += '|'
        #     for y in range(23):
        #         if obs[2][ x][y] >0:
        #             tmp += (f'{obs[2][x][y]}')
        #         else:
        #             tmp += '_'
        #     tmp += '|\n'
        
        # tmp += 25 * '-' + '\n'
        # tmp += f'ACTION: {action.cpu().numpy().flatten()}' + '\n'
        # with open('boards.txt', 'a') as f:
        #     f.write(tmp)
        # print(action)
        # print(value)


    # bench time
    endTime = time.time()
    logger.info(f"Inference took {endTime - startTime} seconds for {id}")

    # convert pytorch tensor to numpy integer
    flattened_action:np.array = action.cpu().numpy().flatten()
    flattened_value = value.cpu().numpy().flatten()
    if flattened_action.shape != (1,) or flattened_value.shape != (1,):
        logger.error(f"Invalid action or value shape: {flattened_action.shape}, {flattened_value.shape}")
        return {
            "action": -1,
            "value": 0.0,
            "error": "Invalid action or value shape",
        }
    if flattened_action[0] not in [0, 1, 2, 3]:
        return {
            "action": -1,
            "value": 0.0,
            "error": "Unexpected network output",
        }

    return {
        "action": int(flattened_action[0]),
        "value": int(flattened_value[0]),
        "error": "",
    }


@app.get("/")
def read_root():
    return {"status": "ok"}

@app.get("/health")
def health():
    return {"status": "ok"}


# TODO: ???
# Record statistics/metrics
