import torch
# from datetime import datetime
import os
from os import path

# FORMULA: The total training set size per iteration is n_envs * n_steps
n_envs = 208     # parallel environments
n_steps = 600    # steps per environment to simulate
n_opponents = 3  # number of opponents to play against

CPU_THREADS = 6
device = torch.device('cuda')
# device = "cpu" if not torch.has_cuda else "cuda:0"

class PathHelper:
    def __init__(self) -> None:
        self.group_name = ''

    def _get_grouppath(self):
        return path.join('models', self.group_name)

    def set_modelgroup(self, name):
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
            if filename.startswith('iter') and filename.endswith('.pt'):
                # filename is iter{iteration}.pt
                try:
                    tmp = int(filename[4:-3])
                    if tmp > iteration:
                        iteration = tmp
                except ValueError:
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


    def get_modelpath(self, iteration=0):
        # t = datetime.now().strftime('%H_%M_%d_%m_%Y')
        # TODO: UUID folder group

        self._prepare_modelgroup_path()
        return path.join('models', self.group_name, f'iter{iteration}.pt')

    def get_modelpath_latest(self):
        self._prepare_modelgroup_path()
        return path.join('models', self.group_name, 'latest.pt')