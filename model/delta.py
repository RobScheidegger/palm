#Import standard packages
from re import T
import numpy as np
import os
import matplotlib.pyplot as plt
# %matplotlib inline
from scipy import io
from scipy import stats
import pickle
import tensorflow as tf # This notebook was tested with TF-2.2.0
import torch
import torch.nn.functional as F

from torch.utils.data import Dataset, DataLoader, TensorDataset

import sys
module_path = os.path.abspath(os.path.join('..'))
if module_path not in sys.path:
    sys.path.append(module_path)
print(module_path)


###################
# HYPERPARAMETERS
LDFL = module_path+'/Decoding_Data/M1_spk_data_extended.mat'
BATCH_SIZE = 100
NUM_CLASSES = 20
EPOCHS = 500
LEARNING_RATE = 1e-3
DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
if torch.cuda.is_available():
    NUM_WORKERS = 8
else:
    NUM_WORKERS = 0
##################

trained_folder = module_path+"Training/"
training_set = ...
testing_set = ...

train_loader = DataLoader(dataset=training_set, batch_size=BATCH_SIZE, shuffle=True, num_workers=8)
test_loader  = DataLoader(dataset=testing_set , batch_size=BATCH_SIZE, shuffle=True, num_workers=8)

for epoch in range(EPOCHS):
    for i, (input, label) in enumerate(train_loader): # training loop
        # print("Input: ", input)
        # print("Label: ", label)
        output = assistant.train(input, label) # set up training
    print(f'\r[Epoch {epoch:2d}/{EPOCHS}] {stats}', end='')
        
    for i, (input, label) in enumerate(test_loader): # training loop
        output = assistant.test(input, label) # set up testing
    print(f'\r[Epoch {epoch:2d}/{EPOCHS}] {stats}', end='')
        
    if epoch%10 == 9: # cleanup display
        print('\r', ' '*len(f'\r[Epoch {epoch:2d}/{EPOCHS}] {stats}'))
        stats_str = str(stats).replace("| ", "\n")
        print(f'[Epoch {epoch:2d}/{EPOCHS}]\n{stats_str}')
    
    if stats.testing.best_accuracy:
        torch.save(net.state_dict(), trained_folder + '/network.pt')
    stats.update()
    stats.save(trained_folder + '/')

stats.plot(figures=(1,2),figsize=(15, 5), path=trained_folder + '/figures/')
plt.close('all')

