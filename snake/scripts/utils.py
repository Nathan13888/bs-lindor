import torch
# from datetime import datetime
import json
import os
from os import path
import matplotlib.pyplot as plt

# TODO: move to config
# FORMULA: The total training set size per iteration is n_envs * n_steps
n_envs = 208     # parallel environments
n_steps = 600    # steps per environment to simulate
n_opponents = 3  # number of opponents to play against

CPU_THREADS = 6 # TODO: configure
# TODO: multigpu support
device = torch.device('cuda')
# device = "cpu" if not torch.has_cuda else "cuda:0"


def plot_graphs(rewards, value_losses, lengths):
    plt.clf()

    plt.title("Average Length vs Episode")
    plt.ylabel("Length")
    plt.xlabel("Iteration")
    plt.plot(lengths)
    plt.show()

    plt.title("Average Loss vs Episode")
    plt.ylabel("Length")
    plt.xlabel("Iteration")
    plt.plot(lengths)
    plt.show(value_losses)

    plt.title("Average Reward vs Episode")
    plt.ylabel("Reward")
    plt.xlabel("Iteration")
    plt.plot(rewards)
    plt.show()

class PathHelper:
    def __init__(self) -> None:
        self.group_name = ''

    def _get_grouppath(self):
        return path.join('models', self.group_name)

    def set_modelgroup(self, name, read_tmp=False):
        self.read_tmp = read_tmp
        self.group_name = name

    # def get_models(name):
    #     # TODO: return model names that is indexed
    #     pass

    # get the latest model in the models folder and find the latest iteration
    def get_latest_model(self):
        latest_model_path = self.get_modelpath_latest()
        iteration = 0

        # get the iteration number from the filename
        for filename in os.listdir(self._get_grouppath()):
            if filename.endswith('.pt'):
                file = ''
                if filename.startswith('iter'):
                    # filename is iter{iteration}.pt
                    file = filename[4:-3]
                elif (self.read_tmp and filename.startswith('tmp_iter')):
                    # filename is tmp_iter{iteration}.pt
                    file = filename[8:-3]
                else:
                    continue

                try:
                    tmp = int(file)
                    if tmp > iteration:
                        iteration = tmp
                except ValueError:
                    print(f'WARNING: {filename} is not a valid model name.')
                    continue

        # there is a latest model
        if path.isfile(latest_model_path):
            # if the latest model has a newer modified date than the latest iteration
            if path.isfile(self.get_modelpath(iteration)):
                if (path.getmtime(latest_model_path) > path.getmtime(self.get_modelpath(iteration))):
                    return latest_model_path, iteration
                # iteration is not the latest model
            else: # iteration does not exist
                return latest_model_path, iteration

        # there is no latest model
        if iteration > 0 and path.isfile(self.get_modelpath(iteration)):
            return self.get_modelpath(iteration), iteration
        
        # there are no models
        return None, iteration

    #
    def _prepare_modelgroup_path(self):
        if len(self.group_name) == 0:
            raise Exception('utils: group_name not set')
        
        # create folder if it doesn't exist
        os.makedirs(self._get_grouppath(), exist_ok=True)


    def get_modelpath(self, iteration=0, custom='', ext='pt'):
        # t = datetime.now().strftime('%H_%M_%d_%m_%Y')
        # TODO: UUID folder group

        if len(custom) == 0:
            custom = 'iter'
        if iteration > 0:
            custom += str(iteration)
        self._prepare_modelgroup_path()
        return path.join('models', self.group_name, f'{custom}.{ext}')

    def get_modelpath_latest(self):
        return self.get_modelpath(custom='latest')
    

    def load_data(self, datafile=None):
        if datafile is None:
            datafile = self.get_modelpath(custom='data', ext='json')

        # read rewards, value_losses, lengths from json file
        if path.isfile(datafile):
            print('Loading data.')
            with open(datafile, 'r') as f:
                data = json.load(f)
                rewards = data['rewards']
                value_losses = data['value_losses']
                lengths = data['lengths']
        else:
            print('No data file found. Starting from scratch.')
            rewards = []
            value_losses = []
            lengths = []
        return rewards, value_losses, lengths


    def save_data(self, rewards, value_losses, lengths, datafile=None):
        if datafile is None:
            datafile = self.get_modelpath(custom='data', ext='json')
            
        # save rewards, value_losses, lengths in json file
        print('Saving data.')
        with open(datafile, 'w') as f:
            json.dump({
                'rewards': rewards,
                'value_losses': value_losses,
                'lengths': lengths,
            }, f)