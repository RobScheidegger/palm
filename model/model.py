import h5py
import torch
from torch.nn import LSTM, Linear, Dropout, ReLU, Sequential, Module, Conv1d, MaxPool1d, Flatten
import torch.nn.functional as F

class DeltaNetwork(torch.nn.Module):

    def __init__(self):
        super(DeltaNetwork, self).__init__()

        self.blocks = torch.nn.ModuleList([# sequential network blocks 
            LSTM(1, 100, batch_first=True),
            Linear(100, 20)
        ])

    def forward(self, x):
        for block in self.blocks:
            spike = block(x)
        return spike