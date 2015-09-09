import time
import paramiko
import socket
import warnings

class Test_Router:
    def __init__(self, ip_addr):
        self.ip_address = ip_addr

    # Bring up connection to the router
    def open(self, use_user='root', use_passw='hydrix1044a'):
        self.username = use_user
        self.password = use_passw
        print 'starting ' + self.username + '@' + self.ip_address
        self.client = paramiko.SSHClient()
        self.client.get_host_keys()
        self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        print 'connecting'
        try:
            self.client.connect(self.ip_address, username=self.username, password=self.password)
        except socket.error:
            print 'connect failed, socket.error!'
            return False
        except paramiko.ssh_exception.BadHostKeyException:
            print 'connect failed, BadHostKeyException!'
            return False
        except paramiko.ssh_exception.AuthenticationException:
            print 'connect failed, AuthenticationException!'
            return False
        except paramiko.ssh_exception.SSHException:
            print 'connect failed, SSHException!'
            return False
        except:
            print 'connect failed!'
            return False
        print 'opening'
        try:
            self.channel = self.client.invoke_shell()
        except:
            print 'open failed!'
            return False
        print 'open'
        return True

    def close(self):
        print 'closing'
        self.client.close()
        print 'closed'

    def do_cmd(self, cmd, timeout=1):
        self.channel.send(cmd + '\n')
        time.sleep(timeout)
        output = self.channel.recv(2345)
        clean = '\n'
        margin = ' -- --: '
        pretty = margin
        for ch in output:
            if ch == '\n':
               pass
            elif ch == '\r':
               clean = clean + '\n'
               pretty = pretty + '\n' + margin
            elif ch < ' ':
               clean = clean + '^' + chr(ord(ch) + 64)
               pretty = pretty + '^' + chr(ord(ch) + 64)
            else:
               clean = clean + str(ch)
               pretty = pretty + str(ch)
        print pretty + '\n'
        return clean
