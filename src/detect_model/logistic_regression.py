import argparse
import csv
import torch
import torch.nn as nn
import torch.optim as optim

# logistic regression

class LogisticRegression(nn.Module):
    def __init__(self):
        super(LogisticRegression, self).__init__()
        self.fc1 = nn.Linear(6, 128)
        self.fc2 = nn.Linear(128, 64)
        self.fc3 = nn.Linear(64, 32)
        self.fc4 = nn.Linear(32, 16)
        self.fc5 = nn.Linear(16, 1)
        self.dropout = nn.Dropout(0.01)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.relu(self.fc3(x))
        x = torch.relu(self.fc4(x))
        x = self.dropout(x)
        x = torch.sigmoid(self.fc5(x))
        return x

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--data_path', type=str, required=True, help='path to data')
    args = parser.parse_args()
    
    data_path: str = args.data_path
    input_data = []
    labels = []
    with open(data_path, 'r') as f:
        csvreader = csv.reader(f)
        next(csvreader)
        for row in csvreader:
            input_data.append([float(x) for x in row[1:]])
            labels.append([float(row[0])])
    train_data = torch.tensor(input_data[:int(len(input_data)*0.8)])
    train_labels = torch.tensor(labels[:int(len(labels)*0.8)])
    test_data = torch.tensor(input_data[int(len(input_data)*0.8):])
    test_labels = torch.tensor(labels[int(len(labels)*0.8):])

    # # training data and label
    # train_data = torch.randn(1000, 6)  # 1000 rand vector 6
    # train_labels = torch.randint(0, 2, (1000, 1)).float()  # 1000 rand labels 0 or 1

    # # test data and label
    # test_data = torch.randn(200, 6)  # 2000 rand vector 6
    # test_labels = torch.randint(0, 2, (200, 1)).float()  # 200 rand labels 0 or 1

    # create model
    model = LogisticRegression()

    # define loss function and optimizer
    criterion = nn.BCELoss()  # cross entropy loss
    # Adam optimizer, L2 regularization (weight_decay parameter)
    optimizer = optim.Adam(model.parameters(), lr=0.03, weight_decay=0.001)

    # train model
    num_epochs = 10  # iteration times
    batch_size = 32  # batch size

    for epoch in range(num_epochs):
        epoch_loss = 0.0
        num_batches = len(train_data) // batch_size

        for batch in range(num_batches):
            start = batch * batch_size
            end = start + batch_size

            # get current batch data and label
            batch_data = train_data[start:end]
            batch_labels = train_labels[start:end]

            # forward propagation
            output = model(batch_data)

            # Calculate loss
            loss = criterion(output, batch_labels)
            epoch_loss += loss.item()

            # backward propagation and optimization
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

        # print loss of each epoch
        print(f"Epoch [{epoch+1}/{num_epochs}], Loss: {epoch_loss/num_batches:.4f}")

    # evaluation mode on test data
    model.eval()
    with torch.no_grad():
        test_output = model(test_data)
        test_pred = (test_output >= 0.5).float()
        accuracy = (test_pred == test_labels).float().mean()
        print(f"Accuracy on test set: {accuracy.item()*100:.2f}%")

if __name__ == '__main__':
    main()
