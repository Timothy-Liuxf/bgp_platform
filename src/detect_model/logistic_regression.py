import argparse
import csv
import random
import torch
import torch.nn as nn
import torch.optim as optim

# logistic regression

class LogisticRegression(nn.Module):
    def __init__(self):
        super(LogisticRegression, self).__init__()
        self.fc1 = nn.Linear(6, 16)
        # self.fc2 = nn.Linear(128, 64)
        # self.fc3 = nn.Linear(64, 32)
        self.fc4 = nn.Linear(16, 8)
        self.fc5 = nn.Linear(8, 1)
        self.dropout = nn.Dropout(0.01)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        # x = torch.relu(self.fc2(x))
        # x = torch.relu(self.fc3(x))
        x = torch.relu(self.fc4(x))
        x = self.dropout(x)
        x = torch.sigmoid(self.fc5(x))
        return x

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--data_path', type=str, required=True, help='path to data')
    args = parser.parse_args()

    print(f"Using data: {args.data_path}")
    
    data_path: str = args.data_path
    input_data = []
    labels = []
    with open(data_path, 'r') as f:
        csvreader = csv.reader(f)
        next(csvreader)
        for row in csvreader:
            input_data.append([float(x) for x in row[1:]])
            labels.append([float(row[0])])
    train_data = input_data[:int(len(input_data)*0.8)]
    train_labels = labels[:int(len(labels)*0.8)]
    test_data = torch.tensor(input_data[int(len(input_data)*0.8):])
    test_labels = torch.tensor(labels[int(len(labels)*0.8):])

    # 提取出相同数量的 train_label 为 0 和 1 的训练集
    new_train_data = []
    new_train_labels = []
    # 将 label 为 1 的全部取出来
    for i in range(len(train_labels)):
        if train_labels[i][0] == 1:
            new_train_data.append(train_data[i])
            new_train_labels.append([1.0,])
    total_num = len(new_train_labels)

    # 随机取出与 label 为 1 相同数量的 label 为 0 的数据
    tmp_train_data = []
    for i in range(len(train_labels)):
        if train_labels[i][0] == 0:
            tmp_train_data.append(train_data[i])

    # 从 tmp_train_data 中随机选出与 label 为 1 相同数量的数据
    random.shuffle(tmp_train_data)
    for i in range(total_num):
        new_train_data.append(tmp_train_data[i])
        new_train_labels.append([0.0,])
    
    train_data = torch.tensor(new_train_data)
    train_labels = torch.tensor(new_train_labels)

    print("Finished loading data")
    print(f"train_data: {train_data.shape}")

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
    optimizer = optim.Adam(model.parameters(), lr=5e-4, weight_decay=0.001)

    # train model
    num_epochs = 40  # iteration times
    batch_size = 256  # batch size

    print("Start training")

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

    print("Finished training")

    # evaluation mode on test data
    # calculate accuracy, precision, recall, f1-score
    model.eval()
    with torch.no_grad():
        test_output = model(test_data)
        test_pred = (test_output >= 0.5).float()
        accuracy = (test_pred == test_labels).float().mean()
        precision = (test_pred * test_labels).sum() / test_pred.sum()
        recall = (test_pred * test_labels).sum() / test_labels.sum()
        f1_score = 2 * precision * recall / (precision + recall)
        print(f"test_data: {len(test_data)}")
        print(f"test_pred: {test_pred.sum()}")
        print(f"test_labels: {test_labels.sum()}")
        print(f"Accuracy on test set: {accuracy.item()*100:.2f}%")
        print(f"Precision on test set: {precision.item()*100:.2f}%")
        print(f"Recall on test set: {recall.item()*100:.2f}%")
        # label 为 0 的数据中，预测为 0 的比例
        print(f"Negative predictive value on test set: {(1-test_pred).sum()/(1-test_labels).sum()*100:.2f}%")
        # label 为 1 的数据中，预测为 1 的比例
        print(f"Positive predictive value on test set: {(test_pred*test_labels).sum()/test_labels.sum()*100:.2f}%")
        # label 为 0 的数据中，预测为 1 的比例
        print(f"False positive rate on test set: {(test_pred.sum()-(test_pred*test_labels).sum())/(1-test_labels).sum()*100:.2f}%")
        # label 为 1 的数据中，预测为 0 的比例
        print(f"False negative rate on test set: {(test_labels.sum()-(test_pred*test_labels).sum())/test_labels.sum()*100:.2f}%")
        print(f"F1 score on test set: {f1_score.item()*100:.2f}%")

if __name__ == '__main__':
    main()
