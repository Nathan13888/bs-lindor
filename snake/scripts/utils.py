import torch
# from datetime import datetime
import json
import os
from os import path
import matplotlib.pyplot as plt

# TODO: move to config
n_opponents = 3  # number of opponents to play against
# TODO: multigpu support
device = torch.device('cuda')
# device = "cpu" if not torch.has_cuda else "cuda:0"


def plot_graphs(rewards, value_losses, action_losses, dist_entropies, lengths):
    plt.clf()

    plt.title("Average Episode Length")
    plt.ylabel("Length")
    plt.xlabel("Iteration")
    plt.plot(lengths)
    plt.show()

    plt.title("Value Loss per Episode")
    plt.ylabel("Value Loss")
    plt.xlabel("Iteration")
    plt.plot(value_losses)
    plt.show()

    plt.title("Action Loss per Episode")
    plt.ylabel("Action Loss")
    plt.xlabel("Iteration")
    plt.plot(action_losses)
    plt.show()

    plt.title("Entropy of Episode")
    plt.ylabel("Entropy")
    plt.xlabel("Iteration")
    plt.plot(dist_entropies)
    plt.show()

    plt.title("Average Reward of Episode")
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
        self.read_tmp = read_tmp # TODO: refactor to `use_tmp`
        self.group_name = name

    # def get_models(name):
    #     # TODO: return model names that is indexed
    #     pass

    # get the latest model in the models folder and find the latest iteration
    def get_latest_model(self):
        latest_model_path = self.get_modelpath_latest()
        iteration = 0

        # print(f'Looking for latest model in {self._get_grouppath()}')

        is_tmp = False
        # get the iteration number from the filename
        for filename in os.listdir(self._get_grouppath()):
            if filename.endswith('.pt'):
                try:
                    tmp = -1
                    tmp_is_tmp = False
                    if filename.startswith('iter'):
                        # filename is iter{iteration}.pt
                        tmp = int(filename[4:-3])
                        
                    elif (self.read_tmp and filename.startswith('tmp_iter')):
                        # filename is tmp_iter{iteration}.pt
                        tmp = int(filename[8:-3])
                        tmp_is_tmp = True
                    else:
                        continue
                    if tmp > iteration:
                        iteration = tmp
                        is_tmp = tmp_is_tmp
                except ValueError:
                    print(f'WARNING: {filename} is not a valid model name.')
                    continue
        # print(f'Latest iteration is {iteration}.')
        custom_fmt = 'iter' if not is_tmp else 'tmp_iter'
        iter_model_path = self.get_modelpath(iteration=iteration, custom=custom_fmt)

        # there is a latest model
        if path.isfile(latest_model_path):
            # if the latest model has a newer modified date than the latest iteration
            if path.isfile(iter_model_path):
                if (path.getmtime(latest_model_path) > path.getmtime(iter_model_path)):
                    return latest_model_path, iteration
                # iteration is not the latest model
            else: # iteration does not exist
                return latest_model_path, iteration

        # there is no latest model
        if iteration > 0 and path.isfile(iter_model_path):
            return iter_model_path, iteration
        
        # there are no models
        return None, iteration

    #
    def _prepare_modelgroup_path(self):
        if len(self.group_name) == 0:
            raise Exception('utils: group_name not set')
        
        # create folder if it doesn't exist
        os.makedirs(self._get_grouppath(), exist_ok=True)


    def get_modelpath(self, iteration=0, custom='', ext='pt'):
        # generate filename
        if len(custom) == 0:
            custom = 'iter'
        if iteration > 0:
            custom += str(iteration)
        self._prepare_modelgroup_path()
        return path.join('models', self.group_name, f'{custom}.{ext}')

    def get_modelpath_latest(self):
        return self.get_modelpath(custom='latest')
    

    def load_data(self, datafile=None, start_iteration=0):
        if datafile is None:
            datafile = self.get_modelpath(custom='data', ext='json')

        # read rewards, value_losses, lengths from json file
        if path.isfile(datafile):
            # print('Loading data.')
            with open(datafile, 'r') as f:
                data = json.load(f)
                rewards = data['rewards']
                value_losses = data['value_losses']
                action_losses = data['action_losses']
                dist_entropies = data['dist_entropies']
                lengths = data['lengths']
        else:
            # print('No data file found. Starting from scratch.')
            rewards = []
            value_losses = []
            action_losses = []
            dist_entropies = []
            lengths = []

        # return a dict
        # crop the the first (start_iteration - 1) elements
        dic = {
            'rewards': rewards[:start_iteration - 1],
            'value_losses': value_losses[:start_iteration - 1],
            'action_losses': action_losses[:start_iteration - 1],
            'dist_entropies': dist_entropies[:start_iteration - 1],
            'lengths': lengths[:start_iteration - 1],
        }
        assert len(dic['rewards']) == len(dic['value_losses']) == len(dic['action_losses']) == len(dic['dist_entropies']) == len(dic['lengths'])
        assert len(dic['rewards']) <= start_iteration - 1
        
        return dic


    def save_data(self, rewards, value_losses, action_losses, dist_entropies, lengths, iteration, datafile=None):
        if datafile is None:
            datafile = self.get_modelpath(custom='data', ext='json')

        data = {
            'rewards': rewards,
            'value_losses': value_losses,
            'action_losses': action_losses,
            'dist_entropies': dist_entropies,
            'lengths': lengths,
        }
            
        # save data in json file
        # print('Saving data.')
        with open(datafile, 'w') as f:
            json.dump(data, f)

        if self.read_tmp and iteration > 0:
            datafile = self.get_modelpath(custom=f'tmp_data_', iteration=iteration, ext='json')
            with open(datafile, 'w') as f:
                json.dump(data, f)