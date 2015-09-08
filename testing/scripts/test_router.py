import time
import paramiko

class Test_Router:
    def __init__(self, ip_addr):
        self.ip_address = ip_addr

    # Bring up connection to the router
    def open(self):
        print 'starting'
        self.client = paramiko.SSHClient()
        self.client.get_host_keys()
        self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        print 'connecting'
        self.client.connect(self.ip_address, username='root', password='hydrix1044a')
        print 'opening'
        self.channel = self.client.invoke_shell()
        print 'open'

    def close(self):
        print 'closing'
        self.client.close()
        print 'closed'

    def do_cmd(self, cmd, timeout=1):
        self.channel.send(cmd + '\n')
        time.sleep(timeout)
        output = self.channel.recv(2345)
        clean=""
        for ch in output:
            if ch < ' ':
               clean = clean + '^' + chr(ord(ch) + 64)
            else:
               clean = clean + str(ch)
        print '\n\n\n[' + clean + ']\n\n\n'
        return clean
