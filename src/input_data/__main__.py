import os

# get current file path
os.chdir(os.path.dirname(os.path.realpath(__file__)))
os.chdir('../..')

from outage_data import download_outage_data

def main():
    download_outage_data('info/outage_data.csv')

if __name__ == '__main__':
    main()
