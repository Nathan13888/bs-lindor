import torch

# FORMULA: The total training set size per iteration is n_envs * n_steps
n_envs = 208     # parallel environments
n_steps = 600    # steps per environment to simulate

CPU_THREADS = 6
device = torch.device('cuda')
# device = "cpu" if not torch.has_cuda else "cuda:0"

def get_modelgroup(name):
    # TODO: return model names that is indexed
    pass

def get_modelpath(fmt):
    t = datetime.now().strftime('%H_%M_%d_%m_%Y')
    # TODO: UUID folder group
    return 'models/'+(fmt.replace("TIME", t)) + '.pt'