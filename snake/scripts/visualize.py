
import torch
import numpy as np
from tqdm import tqdm
from matplotlib import pyplot as plt
from matplotlib import animation
from IPython.display import HTML
from gym_battlesnake.gymbattlesnake import BattlesnakeEnv


def obs_to_frame(obs):
    ''' Converts an environment observation into a renderable RGB image '''
    # First, let's find the game board dimensions from layer 5
    x_offset, y_offset = 0, 0
    done = False
    for x in range(23):
        if done:
            break
        for y in range(23):
            if obs[0][5][x][y] == 1:
                x_offset = x
                y_offset = y
                done = True
                break
    output = np.zeros((11, 11, 3), dtype=np.uint8)

    # See https://github.com/cbinners/gym-battlesnake/blob/master/gym_battlesnake/src/gamewrapper.cpp#L55 for
    # layer reference
    for x in range(23):
        for y in range(23):
            # Render snake bodies
            if obs[0][1][x][y] == 1:
                output[x-x_offset][y-y_offset] = 255 - 10*(255 - obs[0][2][x][y])
            # Render food
            if obs[0][4][x][y] == 1:
                output[x-x_offset][y-y_offset][0] = 255
                output[x-x_offset][y-y_offset][1] = 255
                output[x-x_offset][y-y_offset][2] = 0
            # Render snake heads as a red pixel
            if obs[0][0][x][y] > 0:
                output[x-x_offset][y-y_offset][0] = 255
                output[x-x_offset][y-y_offset][1] = 0
                output[x-x_offset][y-y_offset][2] = 0
            # Render snake heads
            if obs[0][6][x][y] == 1:
                output[x-x_offset][y-y_offset][0] = 0
                output[x-x_offset][y-y_offset][1] = 255
                output[x-x_offset][y-y_offset][2] = 0

    return output


def visualize_game(policy):
    # To watch a replay, we need an environment
    # Important: Make sure to use fixed orientation during visualization
    playground = BattlesnakeEnv(n_threads=1, n_envs=1, opponents=[policy for _ in range(3)], fixed_orientation=True)

    # Reset the environment 
    obs = playground.reset()

    # Keep track of game frames to render
    video = []

    # Grab a set of frames to render
    with torch.no_grad():
        for i in tqdm(range(300)):
            # Add the rendered observation to our frame stack
            video.append(obs_to_frame(obs))

            # Get the action our policy should take
            _, action, _, _ = policy.act(torch.tensor(obs, dtype=torch.float32).to('cuda'), None, None)

            # Perform our action and update our observation
            obs,_,_,_ = playground.step(action.cpu().squeeze())

    # Render, adapted from here: https://stackoverflow.com/questions/57060422/fast-way-to-display-video-from-arrays-in-jupyter-lab

    video = np.array(video, dtype=np.uint8)
    fig = plt.figure()

    def init():
        im.set_data(video[0,:,:,:])
    def animate(i):
        im.set_data(video[i,:,:,:])
        return im

    im = plt.imshow(video[0,:,:,:])
    plt.close()

    anim = animation.FuncAnimation(fig, animate, init_func=init, frames=video.shape[0], interval=200)
    HTML(anim.to_html5_video())