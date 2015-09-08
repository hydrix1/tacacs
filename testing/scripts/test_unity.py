import time

class Unity:
    def __init__(self):
        self.test_max_length = 0
        self.test_total = 0
        self.test_skips = 0
        self.test_fails = 0
        self.test_depth = 0;
        self.test_group = []
        self.test_names = []
        self.test_results = []
        self.test_context=''

    def mark_time(self):
        print "@@@ 123.456"

    def start(self):
        self.test_depth=0
        self.test_context=""
        print '### TACACS+ TEST SCRIPT STARTING ###\n'
        self.test_total=0
        self.test_fails=0
        self.test_skips=0
        self.test_max_length=0

    def end(self):
        filler = '----'
        while len(filler) < self.test_max_length:
            filler = filler + '-'
        padded = 'Test'
        while len(padded) < self.test_max_length:
            padded = padded + ' '
        print '### TEST SCRIPT FINISHED ###'
        print str(self.test_total) + ' Tests ' + str(self.test_fails) + ' Failures ' + str(self.test_skips) + ' Ignored'
        print 'OK\n'
        print 'Test result summary'
        print '   |--------|-' + filler + '-|'
        print '   | Result | ' + padded + ' |'
        print '   |--------|-' + filler + '-|'
        for idx in range(len(self.test_names)):
            padded = self.test_names[idx]
            while len(padded) < self.test_max_length:
                padded = padded + ' '
            print '   |  ' + self.test_results[idx] + '  | ' + padded + ' |'
        print '   |--------|-' + filler + '-|'
        print 'End of summary\n'

    def start_group(self, group_name):
        self.test_group.append(self.test_context)
        self.test_context = self.test_context + group_name + '/'
        self.test_depth += 1

    def end_group(self):
        if self.test_depth > 0:
            self.test_depth -= 1
            self.test_context = self.test_group.pop()
        else:
            print "Error: too many unity_end_groups!"

    def start_test(self, test_name):
        self.skip_test = False
        self.fail_test = False
        self.test_name = test_name
        self.mark_time()
        print '%%% ' + test_name + '\n'

    def end_test(self):
        test_title = self.test_context + self.test_name
        test_title_length = len(test_title)

        if test_title_length > self.test_max_length:
            self.test_max_length = test_title_length

        if self.skip_test:
            test_result = 'IGNORED'
            self.test_skips += 1
        elif self.fail_test:
            test_result = 'FAIL'
            self.test_fails += 1
        else:
            test_result = 'PASS'

        self.test_names.append(test_title)
        self.test_results.append(test_result)
        self.test_total += 1
        self.mark_time()
        print ':::' + test_title + '::: ' + test_result + '\n'

    def skip(self):
        self.skip_test = True

    def fail(self):
        self.fail_test = True


    def assert_equal(self, wanted, got):
        if wanted != got:
            self.fail()
            print ' -- Wanted "' + wanted + '", got "' + got + '"!\n'
