import keras
from keras.models import Sequential
from keras.optimizers import Adam
from keras.layers import Input, Dense, Dropout, Flatten, SimpleRNN, GRU, LSTM, Reshape, BatchNormalization

model = keras.models.Sequential() 
model.add(Reshape((10, 30), input_shape = (10*30, )))
model.add(LSTM(units=10, return_sequences=True) )
model.add(Flatten())
model.add(Dense(64, activation = 'tanh')) 
model.add(Dense(2, activation = 'sigmoid')) 
model.compile(loss = 'binary_crossentropy', optimizer = Adam(lr = 0.001), metrics = ['accuracy'])
modelName = 'model_LSTM.h5'
model.save(modelName)
model.summary()
