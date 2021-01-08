import re
import sys

log_folder = '..\\logs\\'

EXPECTED_LOG = 'expected.log'
TEST_LOG = 'cpu.log'
REGEXES = [
    '(?P<value>^\w{4}  [0-9A-F]{2})', # PC + instruction byte
    'A:(?P<value>[0-9A-F]{2})', # Acumulator
    'X:(?P<value>[0-9A-F]{2})', # X register
    'Y:(?P<value>[0-9A-F]{2})', # Y register
    ' P:(?P<value>[0-9A-F]{2})', # CPU status (P register)
    'SP:(?P<value>[0-9A-F]{2})', # Stack pointer (SP)
    # 'CYC:(?P<value>\d+)', # Cycle
]

def main():
    global log_folder
    
    if len(sys.argv) == 2:
        log_folder = sys.argv[1]

    line_number = 0
    expected = open(log_folder + EXPECTED_LOG, 'r')
    test = open(log_folder + TEST_LOG, 'r')

    expected_line = ''
    test_line = ''

    while True:
        line_number += 1
        expected_line = expected.readline()
        test_line = test.readline()

        # Ignore lines that start with tabs, are meant for debug
        while test_line.startswith('\t'):
            test_line = test.readline()

        if len(expected_line) == 0 and len(test_line) == 0:
            break
            
        if len(expected_line) == 0 or len(test_line) == 0:
            print('Error, found mismatched log at line ' + str(line_number))
            print()
            print('EXPECTED:')
            print(expected_line)
            print('GOT:')
            print(test_line)

            break
        
        for regex in REGEXES:
            expected_match = re.search(regex, expected_line)
            test_match = re.search(regex, test_line)

            if expected_match == None or test_match == None:
                print('Error, something broke at line ' + str(line_number))
                print()
                print('EXPECTED:')
                print(expected_line)
                print('GOT:')
                print(test_line)
                
                break

            if expected_match.group('value') != test_match.group('value'):
                print('Error, found mismatched log at line ' + str(line_number))
                print()
                print('EXPECTED:')
                print(expected_line)
                print('GOT:')
                print(test_line)

                return

    print('Passed successfully')

if __name__ == '__main__':
    main()