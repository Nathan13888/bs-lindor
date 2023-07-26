## Setup

<!-- TODO: gymnasium, SB3 -->
<!-- pip install "stable_baselines3[extra,tests,docs] @ git+https://github.com/DLR-RM/stable-baselines3"         # https://stable-baselines3.readthedocs.io/en/master/guide/install.html -->
```
sudo apt-get update && sudo apt-get install cmake libopenmpi-dev python3-dev zlib1g-devsudo libstdc++6

# setup environment
mamba create --name bs python=3.7
mamba activate bs

mamba install -c conda-forge numpy gym matplotlib tqdm ipykernel ipywidgets  -y
pip install torch==1.13.1+cu117 torchvision==0.14.1+cu117 torchaudio==0.13.1 --extra-index-url https://download.pytorch.org/whl/cu117

mamba install -c anaconda cudnn=7.3.1 -y
pip install tensorflow-gpu==1.14
pip install stable-baselines
mamba install protobuf=3.20 -y

pip install fastapi uvicorn[standard]


cd pytorch-a2c-ppo-acktr-gail/ && pip install -e . && cd ..
cd bs-gym/ && pip install -e . && cd ..


```
<!--
#mamba install pylint -y
#mamba install pytorch==2.0.0 torchvision==0.15.0 torchaudio==2.0.0 pytorch-cuda=11.8 -c pytorch -c nvidia
#mamba install pytorch==1.12.1 torchvision==0.13.1 torchaudio==0.12.1 cudatoolkit=10.2 -c pytorch -y
#mamba install pytorch torchvision torchaudio pytorch-cuda=11.8 -c pytorch -c nvidia -y
#export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python
-->

## Running

Dev:
```
uvicorn inference:app --reload --host 0.0.0.0 --port 7801
```

Prod:
```
(use Docker)
# OR

uvicorn inference:app --host 0.0.0.0 --port 7801
```

<!--## Features-->
