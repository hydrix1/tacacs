import sys
import string
import getopt
import time
import paramiko
import test_unity
import test_router

##################################################################################################

def tacacs_test_cisco_connect(my_unity, my_router):
    output03 = ''
    delay_for = -1
    while output03 == '':
        if delay_for > 0:
            dummy_ssh = test_router.Test_Router(my_router.ip_address)
            dummy_ssh.open("virl", "VIRL")
            out01 = dummy_ssh.do_cmd('')
            out02 = dummy_ssh.do_cmd('telnet '+my_router.ip_address+' '+my_router.indirect, 10)
            out03 = dummy_ssh.do_cmd('', 3, True)
            dummy_ssh.close()
            time.sleep(delay_for)
            my_router.close()
            time.sleep(delay_for)
        my_router.open("virl", "VIRL")
        output01 = my_router.do_cmd('')
        my_unity.expect_text(output01, 'virl@virl')
        output02 = my_router.do_cmd('telnet '+my_router.ip_address+' '+my_router.indirect, 10)
        my_unity.expect_text(output02, 'Connected to')
        output03 = my_router.do_cmd('', 3, True)
        while 'Connection closed by foreign host' in output03:
            output02 = my_router.do_cmd('telnet '+my_router.ip_address+' '+my_router.indirect, 10)
            my_unity.expect_text(output02, 'Connected to')
            output03 = my_router.do_cmd('', 3)
        delay_for += 2
    if "Username:" in output03:
        output03 = my_router.do_cmd('cisco', 3)
    if "Password:" in output03:
        output03 = my_router.do_cmd('cisco', 9)
    if "Username:" in output03:
        output03 = my_router.do_cmd('cisco', 3)
    if "Password:" in output03:
        output03 = my_router.do_cmd('cisco', 9)
    return output03

##################################################################################################

def tacacs_test_cisco_configure_classic(my_unity, my_router, platform, port, key):
    output03 = tacacs_test_cisco_connect(my_unity, my_router)
    if my_router.name+'>'in output03:
        output04 = my_router.do_cmd('enable')
        my_unity.expect_text(output04, 'Password:')
        output03 = my_router.do_cmd('cisco')
    my_unity.expect_text(output03, my_router.name+'#')
    output11 = my_router.do_cmd('terminal length 0', 2)
    my_unity.expect_text(output11, my_router.name+'#')
    output14 = my_router.do_cmd('configure terminal', 2)
    my_unity.expect_text(output14, my_router.name+'(config)#')
    output17 = my_router.do_cmd('ip route 192.168.0.0 255.255.254.0 172.16.1.254')
    my_unity.expect_text(output17, my_router.name+'(config)#')
    output21 = my_router.do_cmd('aaa new-model')
    output22 = my_router.do_cmd('aaa authentication login default line')
    output23 = my_router.do_cmd('aaa authentication login '+my_router.test_name+' group tacacs+ local')
    output24 = my_router.do_cmd('aaa authorization exec '+my_router.test_name+' group tacacs+ none')
    output25 = my_router.do_cmd('aaa authorization commands 15 '+my_router.test_name+' group tacacs+ none')
    output26 = my_router.do_cmd('aaa accounting exec '+my_router.test_name+' start-stop group tacacs+')
    output27 = my_router.do_cmd('aaa accounting commands 15 '+my_router.test_name+' start-stop group tacacs+')
 
    output31 = my_router.do_cmd('tacacs server Jimmy', 3)
    my_unity.expect_text(output31, my_router.name+'(config-server-tacacs)#')
    output32 = my_router.do_cmd('port '+port)
    output33 = my_router.do_cmd('address ipv4 '+platform)
    output34 = my_router.do_cmd('key '+key)
    output35 = my_router.do_cmd('exit')
    my_unity.expect_text(output35, my_router.name+'(config)#')
 
    output41 = my_router.do_cmd('line vty 0 4')
    my_unity.expect_text(output41, my_router.name+'(config-line)#')
    output42 = my_router.do_cmd('authorization commands 15 '+my_router.test_name)
    output43 = my_router.do_cmd('authorization exec '+my_router.test_name)
    output44 = my_router.do_cmd('accounting commands 15 '+my_router.test_name)
    output45 = my_router.do_cmd('accounting exec '+my_router.test_name)
    output46 = my_router.do_cmd('login authentication '+my_router.test_name)
    output49 = my_router.do_cmd('exit')
    my_unity.expect_text(output49, my_router.name+'(config)#')

    output70 = my_router.do_cmd('exit')
    my_unity.expect_text(output70, my_router.name+'#')
    output71 = my_router.do_cmd('exit')
    my_router.close()

##################################################################################################

def tacacs_test_cisco_configure_ios_xr(my_unity, my_router, platform, port, key):
    output = tacacs_test_cisco_connect(my_unity, my_router)
    if my_router.name+'#' not in output:
        if 'Username:' not in output:
            output = my_router.do_cmd('', 2)
    if my_router.name+'#' not in output:
        if 'Username:' not in output:
            output = my_router.do_cmd('', 2)
    if my_router.name+'#' not in output:
        if 'Username:' not in output:
            output = my_router.do_cmd('', 2)
    if my_router.name+'#' not in output:
        if 'Username:' not in output:
            my_router.close()
            return
    if 'Username:' in output:
        output = my_router.do_cmd('cisco')
        my_unity.expect_text(output, 'Password:')
        output = my_router.do_cmd('cisco', 2)
    my_unity.expect_text(output, my_router.name+'#')

    # Rollback to a clean start
    #output = my_router.do_cmd('configure terminal', 2)
    #output = my_router.do_cmd('load bootflash:/base-config', 4)
    #output = my_router.do_cmd('commit', 4)
    #output = my_router.do_cmd('exit')
    output = my_router.do_cmd('copy bootflash:/base-config running-config', 8)

    output = my_router.do_cmd('terminal length 0')
    my_unity.expect_text(output, my_router.name+'#')
    output = my_router.do_cmd('terminal width 0')
    my_unity.expect_text(output, my_router.name+'#')
 
    output = my_router.do_cmd('configure terminal', 3)
    my_unity.expect_text(output, my_router.name+'(config)#')
    output = my_router.do_cmd('ip route 192.168.0.0 255.255.254.0 172.16.1.254', 3)
 
    # add desired server
    output = my_router.do_cmd('tacacs-server host '+platform+' port '+port+' key '+key)

    output = my_router.do_cmd('ip route 192.168.0.0 255.255.254.0 172.16.1.254', 3)
    output = my_router.do_cmd('aaa accounting exec default start-stop group tacacs+', 2)
    output = my_router.do_cmd('aaa accounting system default start-stop group tacacs+', 2)
    output = my_router.do_cmd('aaa accounting network default start-stop group tacacs+', 2)
    output = my_router.do_cmd('aaa accounting commands default start-stop group tacacs+', 2)
    output = my_router.do_cmd('aaa authorization exec default local group tacacs+', 2)
    output = my_router.do_cmd('aaa authorization network default group tacacs+ local', 2)
    output = my_router.do_cmd('aaa authorization commands default group tacacs+ none', 2)
    output = my_router.do_cmd('aaa authorization eventmanager default group tacacs+ local', 2)
    output = my_router.do_cmd('aaa authentication login default local group tacacs+', 2)

    output = my_router.do_cmd('commit', 9)
    my_unity.expect_text(output, my_router.name+'(config)#')
    output = my_router.do_cmd('exit')
    my_unity.expect_text(output, my_router.name+'#')
    output = my_router.do_cmd('ping '+platform, 15)
    my_unity.expect_text(output, my_router.name+'#')
    output = my_router.do_cmd('show config running', 20)
    my_unity.expect_text(output, my_router.name+'#')
    output = my_router.do_cmd('exit')
    my_router.new_line = ''
    output = my_router.do_cmd(chr(29))
    my_router.new_line = '\r'
    output = my_router.do_cmd('close', 2)
    my_router.close()

##################################################################################################

def tacacs_test_cisco_configure_ios_xe(my_unity, my_router, platform, port, key):
    output = tacacs_test_cisco_connect(my_unity, my_router)
    if my_router.name+'>' not in output and my_router.name+'#' not in output:
        if 'Username:' not in output:
            output = my_router.do_cmd('', 2)
    if my_router.name+'>' not in output and my_router.name+'#' not in output:
        if 'Username:' not in output:
            output = my_router.do_cmd('', 2)
    if my_router.name+'>' not in output and my_router.name+'#' not in output:
        if 'Username:' not in output:
            output = my_router.do_cmd('', 2)
    if my_router.name+'>' not in output and my_router.name+'#' not in output:
        if 'Username:' not in output:
            my_router.close()
            return
    if 'Username:' in output:
        output = my_router.do_cmd('cisco')
        my_unity.expect_text(output, 'Password:')
        output = my_router.do_cmd('cisco', 2)

    if my_router.name+'>' in output:
        output = my_router.do_cmd('enable')
        my_unity.expect_text(output, 'Password:')
        output = my_router.do_cmd('cisco', 2)
    my_unity.expect_text(output, my_router.name+'#')

    output = my_router.do_cmd('terminal length 0', 2)
    my_unity.expect_text(output, my_router.name+'#')
    output = my_router.do_cmd('terminal width 0', 2)
    my_unity.expect_text(output, my_router.name+'#')

    output = my_router.do_cmd('configure terminal', 2)
    my_unity.expect_text(output, my_router.name+'(config)#')

    output = my_router.do_cmd('ip route 192.168.0.0 255.255.254.0 172.16.1.254', 3)
    my_unity.expect_text(output, my_router.name+'(config)#')

    output = my_router.do_cmd('prompt %h%p')
    my_unity.expect_text(output, my_router.name+'(config)#')
    output = my_router.do_cmd('username cisco password cisco', 2)
    my_unity.expect_text(output, my_router.name+'(config)#')

    output = my_router.do_cmd('line vty 0 4')
    my_unity.expect_text(output, my_router.name+'(config-line)#')
    #output = my_router.do_cmd('login local')
    my_unity.expect_text(output, my_router.name+'(config-line)#')
    output = my_router.do_cmd('exit')
    my_unity.expect_text(output, my_router.name+'(config)#')

    output = my_router.do_cmd('aaa new-model')
    output = my_router.do_cmd('aaa accounting exec default start-stop group tacacs+')
    output = my_router.do_cmd('aaa accounting system default start-stop group tacacs+')
    output = my_router.do_cmd('aaa accounting network default start-stop group tacacs+')
    for lvl in range(16):
        output = my_router.do_cmd('aaa accounting commands '+str(lvl)+' default start-stop group tacacs+')
    output = my_router.do_cmd('aaa authorization exec default group tacacs+ local')
    output = my_router.do_cmd('aaa authorization network default group tacacs+ local')
    for lvl in range(16):
        output = my_router.do_cmd('aaa authorization commands '+str(lvl)+' default group tacacs+ none')
    #output = my_router.do_cmd('aaa authorization eventmanager default group tacacs+ local')
    output = my_router.do_cmd('aaa authentication login default group tacacs+ local')
 
    my_unity.expect_text(output, my_router.name+'(config)#')
    output = my_router.do_cmd('tacacs server '+my_router.test_name, 5)
    my_unity.expect_text(output, my_router.name+'(config-server-tacacs)#')
    output = my_router.do_cmd('address ipv4 '+platform)
    my_unity.expect_text(output, my_router.name+'(config-server-tacacs)#')
    output = my_router.do_cmd('port '+port)
    my_unity.expect_text(output, my_router.name+'(config-server-tacacs)#')
    output = my_router.do_cmd('key '+key)
    my_unity.expect_text(output, my_router.name+'(config-server-tacacs)#')
    output = my_router.do_cmd('exit')
    my_unity.expect_text(output, my_router.name+'(config)#')

    output = my_router.do_cmd('exit')
    my_unity.expect_text(output, my_router.name+'#')
    output = my_router.do_cmd('exit')
    if my_router.name+'>' in output:
        output = my_router.do_cmd('exit')
    my_router.close()

##################################################################################################

def tacacs_test_cisco_configure_whatever(my_unity, my_router, platform, port, key):
    if my_router.genre == 'classic':
        tacacs_test_cisco_configure_classic(my_unity, my_router, platform, port, key)
    if my_router.genre == 'IOS-XR':
        tacacs_test_cisco_configure_ios_xr(my_unity, my_router, platform, port, key)
    if my_router.genre == 'IOS-XE':
        tacacs_test_cisco_configure_ios_xe(my_unity, my_router, platform, port, key)

##################################################################################################

def tacacs_test_cisco_open_user(my_unity, my_router, username, password):
    test_ssh = test_router.Test_Router(my_router.ip_address)
    test_ssh.new_line = '\r'
    ssh_good = test_ssh.open("virl", "VIRL")
    is_good = False
    if not ssh_good:
        my_unity.fail(context + ' SSH failed!')
    else:
        output = test_ssh.do_cmd('telnet '+my_router.test_addr, 10)
        my_unity.expect_text(output, 'Connected to')
        if 'Username:' in output:
            output = test_ssh.do_cmd(username)
        if 'Password:' in output:
            output = test_ssh.do_cmd(password, 3)
        if 'Connection closed by foreign host' in output:
            output = test_ssh.do_cmd('telnet '+my_router.test_addr, 10)
            my_unity.expect_text(output, 'Connected to')
        if my_router.genre == 'IOS-XR':
            if 'Username:' in output:
                output = test_ssh.do_cmd(username)
            if 'Password:' in output:
                output = test_ssh.do_cmd(password, 28)
            if 'Connection closed by foreign host' in output:
                output = test_ssh.do_cmd('telnet '+my_router.test_addr, 10)
                my_unity.expect_text(output, 'Connected to')
            if 'Username:' in output:
                output = test_ssh.do_cmd(username)
            if 'Password:' in output:
                output = test_ssh.do_cmd(password, 20)
            if 'Connection closed by foreign host' in output:
                output = test_ssh.do_cmd('telnet '+my_router.test_addr, 10)
                my_unity.expect_text(output, 'Connected to')
            if 'Username:' in output:
                output = test_ssh.do_cmd(username)
            if 'Password:' in output:
                output = test_ssh.do_cmd(password, 40)
            if 'Connection closed by foreign host' in output:
                output = test_ssh.do_cmd('telnet '+my_router.test_addr, 10)
                my_unity.expect_text(output, 'Connected to')
        if 'Username:' in output:
            output = test_ssh.do_cmd(username)
        if 'Password:' in output:
            output = test_ssh.do_cmd(password, 8)
        if my_router.name+'>' in output or my_router.name+'#' in output:
            is_good = True
    return (ssh_good, is_good, test_ssh)

def tacacs_test_cisco_user_int(my_unity, my_router, username, password):
    (ssh_good, telnet_good, trial_ssh) = tacacs_test_cisco_open_user(my_unity, my_router, username, password)
    if telnet_good:
        output = trial_ssh.do_cmd('logout', 3)
    if ssh_good:
        trial_ssh.new_line = ''
        output = trial_ssh.do_cmd(chr(29))
        trial_ssh.new_line = '\r'
        output = trial_ssh.do_cmd('close')
    trial_ssh.close()
    return ssh_good and telnet_good

def tacacs_test_cisco_user(my_unity, my_router, username, password, expect_good):
    context = 'Username "' + username + '"/password "' + password + '"'
    my_unity.start_test('user_' + username + '_pass_' + password)
    was_good = tacacs_test_cisco_user_int(my_unity, my_router, username, password)
    if expect_good and not was_good:
        my_unity.fail(context + ' failed!')
    if not expect_good and was_good:
        my_unity.fail(context + ' unexpectedly worked!')
    my_unity.end_test()

##################################################################################################

def tacacs_test_cisco_comms(my_unity, my_router):
    my_unity.start_test('comms')
    output = tacacs_test_cisco_connect(my_unity, my_router)
    my_unity.expect_text(output, my_router.name)
    output = my_router.do_cmd('exit')
    my_router.new_line = ''
    output = my_router.do_cmd(chr(29))
    my_router.new_line = '\r'
    output = my_router.do_cmd('close')
    my_unity.expect_text(output, 'virl@virl')
    my_router.close()
    my_unity.end_test()

def tacacs_test_cisco_config(my_unity, my_router, platform, port, key, username, password):
    my_unity.start_test('config')
    tacacs_test_cisco_configure_whatever(my_unity, my_router, platform, port, key)
    worked = tacacs_test_cisco_user_int(my_unity, my_router, username, password)
    if not worked:
        my_unity.fail()
        print ' -- basic config sanity check failed!\n'
    my_unity.end_test()

def tacacs_test_cisco_bad_server(my_unity, my_router, platform, port, key, username, password):
    my_unity.start_test('bad_server')
    tacacs_test_cisco_configure_whatever(my_unity, my_router, platform, '666', key)
    worked = tacacs_test_cisco_user_int(my_unity, my_router, username, password)
    if worked:
        my_unity.fail()
        print ' -- user authorised using non-existant server!\n'
    my_unity.end_test()

def tacacs_test_cisco_bad_key(my_unity, my_router, platform, port, key, username, password):
    my_unity.start_test('bad_key')
    tacacs_test_cisco_configure_whatever(my_unity, my_router, platform, port, 'not' + key)
    worked = tacacs_test_cisco_user_int(my_unity, my_router, username, password)
    if worked:
        my_unity.fail()
        print ' -- user authorised using bad secret key!\n'
    my_unity.end_test()

def tacacs_test_cisco_basic(my_unity, my_router, platform, port, secret):
    my_unity.start_group('basic')
    good_username = 'test_'+port+'_a'
    good_password = 'A'+port
    tacacs_test_cisco_comms(my_unity, my_router)
    tacacs_test_cisco_config(my_unity, my_router, platform, port, secret, good_username, good_password)
    tacacs_test_cisco_bad_server(my_unity, my_router, platform, port, secret, good_username, good_password)
    tacacs_test_cisco_bad_key(my_unity, my_router, platform, port, secret, good_username, good_password)
    my_unity.end_group()

##################################################################################################

def tacacs_test_cisco_simple_access_good(my_unity, my_router, port):
    my_unity.start_group('good_access')
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_a', 'A'+port, True)
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_b', 'B'+port, True)
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_b', 'XXXX',   True)
    #tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_b', '',       True)
    my_unity.end_group()

def tacacs_test_cisco_simple_access_bad(my_unity, my_router, port):
    my_unity.start_group('bad_access')
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_a', '',       False)
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_a', 'X'+port, False)
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_c', 'C'+port, False)
    tacacs_test_cisco_user(my_unity, my_router, '',                'X'+port, False)
    tacacs_test_cisco_user(my_unity, my_router, '',                '',       False)
    tacacs_test_cisco_user(my_unity, my_router, 'test_'+port+'_x', 'X'+port, False)
    my_unity.end_group()

def tacacs_test_cisco_simple_access(my_unity, my_router, port):
    tacacs_test_cisco_simple_access_good(my_unity, my_router, port)
    tacacs_test_cisco_simple_access_bad(my_unity, my_router, port)

##################################################################################################

def tacacs_test_cisco_simple_authorisation_start(my_unity, my_router, username, password):
    my_unity.start_group(username)
    (good, open, session) = tacacs_test_cisco_open_user(my_unity, my_router, username, password)
    if good and not open:
        test_ssh.new_line = ''
        output = test_ssh.do_cmd(chr(29))
        test_ssh.new_line = '\r'
        output = test_ssh.do_cmd('close')
    if open:
        session.do_cmd('terminal length 0', 2)
    return (open, session)

def tacacs_test_cisco_cmd(my_unity, my_router, open, session, cmd, expect_pass):
    my_unity.start_test(cmd.replace(' ','_'))
    if open:
        output = session.do_cmd('', 2)
        if my_router.name+'>' in output:
            output = session.do_cmd('enable', 3)
            if 'Password:' in output:
                output = session.do_cmd('cisco', 3)
        #if  my_router.name+'#' in output:
        output = session.do_cmd(cmd, 9)
        if 'authorization failed' in output:
            if expect_pass:
                my_unity.fail(cmd + ' failed!')
        else:
            if not expect_pass:
                my_unity.fail(cmd + ' unexpectedly worked!')
    else:
        my_unity.skip()
    my_unity.end_test()

def tacacs_test_cisco_simple_authorisation_end(my_unity, session):
    session.close()
    my_unity.end_group()

def tacacs_test_cisco_simple_authorisation_a(my_unity, my_router, port):
    (open, session) = tacacs_test_cisco_simple_authorisation_start(my_unity, my_router, 'test_'+port+'_a', 'A'+port)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'delete flash:/gordon', False)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'copy running-config null:', True)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'copy runn null:', True)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'copy startup-config null:', False)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'write terminal', True)
    tacacs_test_cisco_simple_authorisation_end(my_unity, session)

def tacacs_test_cisco_simple_authorisation_b(my_unity, my_router, port):
    (open, session) = tacacs_test_cisco_simple_authorisation_start(my_unity, my_router, 'test_'+port+'_b', 'B'+port)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'delete flash:/gordon', False)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'copy running-config null:', False)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'copy runn null:', False)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'copy startup-config null:', False)
    tacacs_test_cisco_cmd(my_unity, my_router, open, session, 'write terminal', True)
    tacacs_test_cisco_simple_authorisation_end(my_unity, session)

def tacacs_test_cisco_simple_authorisation(my_unity, my_router, port):
    my_unity.start_group('authorisation')
    tacacs_test_cisco_simple_authorisation_a(my_unity, my_router, port)
    tacacs_test_cisco_simple_authorisation_b(my_unity, my_router, port)
    my_unity.end_group()

##################################################################################################

def tacacs_test_cisco_simple(my_unity, my_router, platform, port, name):
    port_str = str(port)
    print ' -- Testing simple ' + name + ' (port ' + port_str + ')'
    my_unity.start_group(name)
    tacacs_test_cisco_configure_whatever(my_unity, my_router, platform, port_str, 'cisco')
    tacacs_test_cisco_simple_access(my_unity, my_router, port_str)
    #tacacs_test_accounting
    tacacs_test_cisco_simple_authorisation(my_unity, my_router, port_str)
    my_unity.end_group()


##################################################################################################

def tacacs_test_cisco_all(my_unity, my_router, platform, genre, name, indirect, address):
    my_router.genre = genre
    my_router.name = name
    my_router.indirect = indirect
    my_router.test_addr = address
    my_router.new_line = '\r'
    my_unity.start_group(my_router.genre)
    tacacs_test_cisco_basic(my_unity, my_router, platform, '4901', 'cisco')
    tacacs_test_cisco_simple(my_unity, my_router, platform, 4901, 'original')
    tacacs_test_cisco_simple(my_unity, my_router, platform, 4902, 'reference')
    tacacs_test_cisco_simple(my_unity, my_router, platform, 4903, 'CLI')
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
    print 'CISCO router is ' + router

    # With exclusive use of CISCO router, run tests
    my_unity = test_unity.Unity()
    my_router = test_router.Test_Router(router)
    my_router.test_name = 'Jimmy'

    # With exclusive use of CISCO router, run tests
    my_unity.start()
    my_unity.start_group("Cisco")
    tacacs_test_cisco_all(my_unity, my_router, platform, 'classic', 'iosv-1',     '17003', '172.16.1.85')
    # XR testing has been removed pro temp
    #tacacs_test_cisco_all(my_unity, my_router, platform, 'IOS-XR',  'iosxrv-1',   '17000', '172.16.1.84')
    tacacs_test_cisco_all(my_unity, my_router, platform, 'IOS-XE',  'csr1000v-1', '17005', '172.16.1.86')

    my_unity.end_group()
    my_unity.end()

if __name__ == "__main__":
   main(sys.argv[0], sys.argv[1:])
