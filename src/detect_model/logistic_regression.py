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
        self.fc1 = nn.Linear(6, 64)
        # self.fc2 = nn.Linear(256, 64)
        # self.fc3 = nn.Linear(64, 16)
        # self.fc4 = nn.Linear(16, 8)
        self.fc5 = nn.Linear(64, 1)
        self.dropout = nn.Dropout(0.1)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        # x = torch.relu(self.fc2(x))
        # x = torch.relu(self.fc3(x))
        # x = torch.relu(self.fc4(x))
        x = self.dropout(x)
        x = torch.sigmoid(self.fc5(x))
        return x


def extract_data(data_path: str, out_train_data_path: str, out_test_data_path: str):
    input_data = []
    labels = []
    print("Reading data...")
    cnt = 0
    with open(data_path, 'r') as f:
        csvreader = csv.reader(f)
        next(csvreader)
        for row in csvreader:
            input_data.append([float(x) for x in row[1:]])
            labels.append([float(row[0])])
            cnt += 1
            if cnt % 1000000 == 0:
                print(f"Reading {cnt} lines...")
    train_data = input_data[:int(len(input_data)*0.8)]
    train_labels = labels[:int(len(labels)*0.8)]
    test_data = input_data[int(len(input_data)*0.8):]
    test_labels = labels[int(len(labels)*0.8):]

    print("start to extract train data")

    # 提取出相同数量的 train_label 为 0 和 1 的训练集
    new_train_data = []
    new_train_labels = []
    # 将 label 为 1 的全部取出来
    for i in range(len(train_labels)):
        if train_labels[i][0] == 1:
            new_train_data.append(train_data[i])
            new_train_labels.append([1.0,])

    # 随机取出与 label 为 1 相同数量的 label 为 0 的数据
    tmp_train_data = []
    for i in range(len(train_labels)):
        if train_labels[i][0] == 0:
            tmp_train_data.append(train_data[i])

    total_num = min(len(new_train_labels), len(tmp_train_data))

    # 从 tmp_train_data 中随机选出与 label 为 1 相同数量的数据
    random.shuffle(tmp_train_data)
    for i in range(total_num):
        new_train_data.append(tmp_train_data[i])
        new_train_labels.append([0.0,])

    # 随机打乱 train_data 和 train_labels
    tmp = list(zip(new_train_data, new_train_labels))
    random.shuffle(tmp)
    new_train_data, new_train_labels = zip(*tmp)

    train_data = new_train_data
    train_labels = new_train_labels

    print("start to extract test data")

    # 提取出相同数量的 test_label 为 0 和 1 的训练集
    new_test_data = []
    new_test_labels = []
    # 将 label 为 1 的全部取出来
    for i in range(len(test_labels)):
        if test_labels[i][0] == 1:
            new_test_data.append(test_data[i])
            new_test_labels.append([1.0,])

    # 随机取出与 label 为 1 相同数量的 label 为 0 的数据
    tmp_test_data = []
    for i in range(len(test_labels)):
        if test_labels[i][0] == 0:
            tmp_test_data.append(test_data[i])

    total_num = min(len(new_test_labels) * 3, len(tmp_test_data))

    # 从 tmp_test_data 中随机选出与 label 为 1 相同数量的数据
    random.shuffle(tmp_test_data)
    for i in range(total_num):
        new_test_data.append(tmp_test_data[i])
        new_test_labels.append([0.0,])

    # 随机打乱 test_data 和 test_labels
    tmp = list(zip(new_test_data, new_test_labels))
    random.shuffle(tmp)
    new_test_data, new_test_labels = zip(*tmp)

    test_data = new_test_data
    test_labels = new_test_labels

    print("start to write data to csv file")

    print("train_data size: ", len(train_data))
    cnt = 0
    with open('train_data.csv', 'w') as f:
        csvwriter = csv.writer(f)
        csvwriter.writerow(['label', 'x1', 'x2', 'x3', 'x4', 'x5', 'x6'])
        for i in range(len(train_data)):
            csvwriter.writerow([train_labels[i][0], train_data[i][0], train_data[i][1],
                               train_data[i][2], train_data[i][3], train_data[i][4], train_data[i][5]])
            cnt += 1
            if cnt % 10000 == 0:
                print(f"Writing {cnt} lines...")

    print("test_data size: ", len(test_data))
    cnt = 0
    with open('test_data.csv', 'w') as f:
        csvwriter = csv.writer(f)
        csvwriter.writerow(['label', 'x1', 'x2', 'x3', 'x4', 'x5', 'x6'])
        for i in range(len(test_data)):
            csvwriter.writerow([test_labels[i][0], test_data[i][0], test_data[i][1],
                               test_data[i][2], test_data[i][3], test_data[i][4], test_data[i][5]])
            cnt += 1
            if cnt % 10000 == 0:
                print(f"Writing {cnt} lines...")


def train(train_data_path: str, test_data_path: str):
    with open(train_data_path, 'r') as f:
        csvreader = csv.reader(f)
        next(csvreader)
        train_data = []
        train_labels = []
        for row in csvreader:
            train_data.append([float(x) for x in row[1:]])
            train_labels.append([float(row[0])])

    train_data = torch.tensor(train_data)
    train_labels = torch.tensor(train_labels)

    with open(test_data_path, 'r') as f:
        csvreader = csv.reader(f)
        next(csvreader)
        test_data = []
        test_labels = []
        for row in csvreader:
            test_data.append([float(x) for x in row[1:]])
            test_labels.append([float(row[0])])

    test_data = torch.tensor(test_data)
    test_labels = torch.tensor(test_labels)

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
    learning_rate = 1e-3
    weight_decay = 1e-4
    optimizer = optim.Adam(model.parameters(), lr=learning_rate, weight_decay=weight_decay)
    # other optimizers
    # optimizer = optim.SGD(model.parameters(), lr=learning_rate, momentum=0.9, weight_decay=weight_decay)

    # train model
    num_epochs = 60  # iteration times
    batch_size = 4096  # batch size

    print("Start training")

    for epoch in range(num_epochs):
        epoch_loss = 0.0
        num_batches = len(train_data) // batch_size

        model.train()
        for batch in range(num_batches):
            start = batch * batch_size
            end = start + batch_size

            # get current batch data and label
            batch_data = train_data[start:end]
            batch_labels = train_labels[start:end]

            # clear gradients of all parameters
            optimizer.zero_grad()

            # forward propagation
            output = model(batch_data)

            # Calculate loss
            loss = criterion(output, batch_labels)
            loss.backward()
            epoch_loss += loss.item()

            # backward propagation and optimization
            # optimizer.zero_grad()
            # loss.backward()
            optimizer.step()

        # print loss of each epoch
        print(
            f"Epoch [{epoch+1}/{num_epochs}], Loss: {epoch_loss/num_batches:.4f}")
        # train accuracy
        # model.eval()
        # with torch.no_grad():
        #     train_output = model(train_data)
        #     train_pred = (train_output >= 0.5).float()
        #     accuracy = (train_pred == train_labels).float().mean()
        #     print(f"Accuracy on train set: {accuracy.item()*100:.2f}%")

    print("Finished training")

    # print model parameters
    print("Model parameters:")
    for name, param in model.named_parameters():
        if param.requires_grad:
            print(name, param.data)

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
        print(f"test_pred*test_labels: {(test_pred*test_labels).sum()}")
        print(f"Accuracy on test set: {accuracy.item()*100:.2f}%")
        print(f"Precision on test set: {precision.item()*100:.2f}%")
        print(f"Recall on test set: {recall.item()*100:.2f}%")
        # label 为 0 的数据中，预测为 0 的比例
        print(
            f"Negative predictive value on test set: {((1-test_pred)*(1-test_labels)).sum()/(1-test_labels).sum()*100:.2f}%")
        # label 为 1 的数据中，预测为 1 的比例
        print(
            f"Positive predictive value on test set: {(test_pred*test_labels).sum()/test_labels.sum()*100:.2f}%")
        # label 为 0 的数据中，预测为 1 的比例
        print(
            f"False positive rate on test set: {(test_pred.sum()-(test_pred*test_labels).sum())/(1-test_labels).sum()*100:.2f}%")
        # label 为 1 的数据中，预测为 0 的比例
        print(
            f"False negative rate on test set: {(test_labels.sum()-(test_pred*test_labels).sum())/test_labels.sum()*100:.2f}%")
        print(f"F1 score on test set: {f1_score.item()*100:.2f}%")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--data_path', type=str,
                        required=False, help='path to data')
    parser.add_argument('--train_data_path', type=str,
                        required=False, help='path to train data')
    parser.add_argument('--test_data_path', type=str,
                        required=False, help='path to test data')
    parser.add_argument('--extract', action='store_true',
                        required=False, help='extract data')
    args = parser.parse_args()

    if args.extract:
        if not args.data_path or not args.train_data_path or not args.test_data_path:
            raise Exception(
                'data_path or train_path or test_data_path is not provided')
        extract_data(args.data_path, args.train_data_path, args.test_data_path)
    else:
        if not args.train_data_path or not args.test_data_path:
            raise Exception(
                'train_data_path or test_data_path is not provided')
        train(args.train_data_path, args.test_data_path)


if __name__ == '__main__':
    main()
