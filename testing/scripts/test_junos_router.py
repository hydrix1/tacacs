import sys
import getopt
import time
import paramiko
import test_unity
import test_router

##################################################################################################

##################################################################################################

def tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, port, key):
    my_router.open();
    output1 = my_router.do_cmd('cli', 2)
    my_unity.expect_text(output1, '\n--- JUNOS ')
    #my_unity.dont_expect_text(output1, 'Password incorrect')
    output2 = my_router.do_cmd('edit')
    output3 = my_router.do_cmd('set system tacplus-server ' + platform + ' port ' + port + ' secret ' + key)
    output4 = my_router.do_cmd('commit', 5)
    output5 = my_router.do_cmd('exit')
    output6 = my_router.do_cmd('exit')
    output7 = my_router.do_cmd('exit')
    my_unity.expect_text(output7, 'logout')
    my_router.close();

##################################################################################################

def tacacs_test_junos_user_int(my_router, username, password):
    test_ssh = test_router.Test_Router(my_router.ip_address)
    is_good = test_ssh.open(username, password)
    test_ssh.close();
    return is_good

def tacacs_test_junos_user(my_unity, my_router, username, password, expect_good):
    context = 'Username "' + username + '"/password "' + password + '"'
    my_unity.start_test('user_' + username + '_pass_' + password)
    test_ssh = test_router.Test_Router(my_router.ip_address)
    is_good = test_ssh.open(username, password)
    if expect_good and not is_good:
        my_unity.fail(context + ' failed!')
    if not expect_good and is_good:
        my_unity.fail(context + ' unexpectedly worked!')
    test_ssh.close();
    my_unity.end_test()

##################################################################################################

def tacacs_test_junos_comms(my_unity, my_router):
    my_unity.start_test('comms')
    my_router.open();
    output = my_router.do_cmd('logout');
    my_unity.expect_text(output, '--- JUNOS ')
    #my_unity.dont_expect_text(output, 'Password incorrect')
    my_unity.expect_text(output, 'logout')
    my_router.close();
    my_unity.end_test()

def tacacs_test_junos_config(my_unity, my_router, platform, port, key, username, password):
    my_unity.start_test('config')
    tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, port, key)
    worked = tacacs_test_junos_user_int(my_router, username, password)
    if not worked:
        my_unity.fail()
        print ' -- basic config sanity check failed!\n'
    my_unity.end_test()

def tacacs_test_junos_bad_server(my_unity, my_router, platform, port, key, username, password):
    my_unity.start_test('bad_server')
    tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, '666', key)
    worked = tacacs_test_junos_user_int(my_router, username, password)
    if worked:
        my_unity.fail()
        print ' -- user authorised using non-existant server!\n'
    my_unity.end_test()

def tacacs_test_junos_bad_key(my_unity, my_router, platform, port, key, username, password):
    my_unity.start_test('bad_key')
    tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, port, 'not' + key)
    worked = tacacs_test_junos_user_int(my_router, username, password)
    if worked:
        my_unity.fail()
        print ' -- user authorised using bad secret key!\n'
    my_unity.end_test()

def tacacs_test_junos_basic(my_unity, my_router, platform, port, secret):
    my_unity.start_group('basic')
    good_username = 'test_'+port+'_a'
    good_password = 'A'+port
    tacacs_test_junos_comms(my_unity, my_router)
    tacacs_test_junos_config(my_unity, my_router, platform, port, secret, good_username, good_password)
    tacacs_test_junos_bad_server(my_unity, my_router, platform, port, secret, good_username, good_password)
    tacacs_test_junos_bad_key(my_unity, my_router, platform, port, secret, good_username, good_password)
    my_unity.end_group()

##################################################################################################

def tacacs_test_junos_simple_access(my_unity, my_router, port):
    my_unity.start_group('good_access')
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_a', 'A'+port, True)
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_b', 'B'+port, True)
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_b', 'XXXX',   True)
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_b', '',       True)
    my_unity.end_group()
    my_unity.start_group('bad_access')
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_a', '',       False)
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_a', 'X'+port, False)
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_c', 'C'+port, False)
    tacacs_test_junos_user(my_unity, my_router, '',                'X'+port, False)
    tacacs_test_junos_user(my_unity, my_router, '',                '',       False)
    tacacs_test_junos_user(my_unity, my_router, 'test_'+port+'_x', 'X'+port, False)
    my_unity.end_group()

def tacacs_test_junos_simple(my_unity, my_router, platform, port, name):
    port_str = str(port)
    print ' -- Testing simple ' + name + ' (port ' + port_str + ')'
    my_unity.start_group(name)
    tacacs_test_junos_configure_tacacs(my_unity, my_router, platform, port_str, 'cisco')
    tacacs_test_junos_simple_access(my_unity, my_router, port_str)
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
    tacacs_test_junos_basic(my_unity, my_router, platform, '4900', 'cisco')
    tacacs_test_junos_simple(my_unity, my_router, platform, 4900, 'reference')
    my_unity.end_group()
    my_unity.end()

if __name__ == "__main__":
   main(sys.argv[0], sys.argv[1:])
