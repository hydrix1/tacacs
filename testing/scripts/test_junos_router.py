import sys
import getopt
import time
import paramiko
import test_unity
import test_router

##################################################################################################

def tacacs_expect_text(my_unity, output, wanted):
    if wanted not in output:
        my_unity.fail()
        print ' -- didn\'t get ' + wanted + '\n'



def tacacs_dont_expect_text(my_unity, output, not_wanted):
    if not_wanted in output:
        my_unity.fail()
        print ' -- got unwanted ' + not_wanted + '\n'

##################################################################################################

def tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, port, key):
    my_router.open();
    output1 = my_router.do_cmd('cli')
    output2 = my_router.do_cmd('edit')
    output3 = my_router.do_cmd('set system tacplus-server ' + platform + ' port ' + port + ' secret ' + key)
    output4 = my_router.do_cmd('commit', 10)
    output5 = my_router.do_cmd('exit')
    output6 = my_router.do_cmd('exit')
    output7 = my_router.do_cmd('exit')
    print '--- ***************************'
    print output1
    print '--- ***************************'
    print output2
    print '--- ***************************'
    print output3
    print '--- ***************************'
    print output4
    print '--- ***************************'
    print output5
    print '--- ***************************'
    print output6
    print '--- ***************************'
    print output7
    print '--- ***************************'
    #tacacs_expect_text "^--- JUNOS "
    #tacacs_dont_expect_text "Password incorrect"
    #tacacs_expect_text "^logout"
    my_router.close();

##################################################################################################

def tacacs_test_junos_comms(my_unity, my_router):
    my_unity.start_test('comms')
    my_router.open();
    output = my_router.do_cmd('logout');
    tacacs_expect_text(my_unity, output, '--- JUNOS ')
    #tacacs_dont_expect_text(my_unity, output, 'Password incorrect')
    tacacs_expect_text(my_unity, output, 'logout')
    my_router.close();
    my_unity.end_test()

##################################################################################################

def tacacs_test_junos_simple_access(my_unity, my_router, platform, port):
    my_unity.start_test('access')
    tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, port, 'cisco')
    #tacacs_test_junos_user "test_${tacacs_port}_a" "A${tacacs_port}"
    #tacacs_test_junos_user "test_${tacacs_port}_b" "B${tacacs_port}"
    #tacacs_test_junos_user "test_${tacacs_port}_c" "C${tacacs_port}"
    #tacacs_test_junos_user "test_${tacacs_port}_x" "X${tacacs_port}"
    my_unity.end_test()

def tacacs_test_junos_simple(my_unity, my_router, platform, port, name):
    port_str = str(port)
    print ' -- Testing simple ' + name + ' (port ' + port_str + ')'
    my_unity.start_group(name)
    tacacs_test_junos_simple_access(my_unity, my_router, platform, port_str)
    #tacacs_test_accounting
    #tacacs_test_authorisation
    my_unity.end_group()


##################################################################################################

def main(prog, argv):
    platform = ''
    router = ''

    # Extract command line args
    try:
        opts, args = getopt.getopt(argv, "hp:r:",["platform=","router="])
    except getopt.GetoptError:
        print prog + ' -p <platform> -r <router>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'test.py -p <platform> -r <router>'
            sys.exit()
        elif opt in ("-p", "--platform"):
            platform = arg
        elif opt in ("-r", "--router"):
           router = arg
    print 'TACACS+ server platform is ' + platform
    print 'JUNOS router is ' + router

    # With exclusive use of JUNOS router, run tests
    my_unity = test_unity.Unity()
    my_router = test_router.Test_Router(router)

    # With exclusive use of JUNOS router, run tests
    my_unity.start()
    my_unity.start_group("JunOS")
    tacacs_test_junos_comms(my_unity, my_router)
    tacacs_test_junos_simple(my_unity, my_router, platform, 4900, "reference")
    my_unity.end_group()
    my_unity.end()

if __name__ == "__main__":
   main(sys.argv[0], sys.argv[1:])
