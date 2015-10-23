# Command-Line TacPlus User Guide

* Author: Geoff Wong (geoff.wong@hydrix.com)
* Status: Draft
* Date: 23/Oct/2015

## Overview

In addition to the normal tac_plus configuration files, the modified tac_plus executable supports a
set of command line options to configure the the tac_plus server for operation.

Options are primary specified using GNU longopt style options (ie. --option). The order of options is
important.

## Options

        Usage: tac_plus [ <Options> ] [ <configuration file> ] [ <id> ]

        Options:
          -P, --check                            parse configuration file, then quit
          -l, --degraded                         enable single-process ("degraded") mode
          -v, --version                          show version, then quit
          -b, --background                       force going to background
          -f, --foreground                       force staying in foreground
          -i, --child-id=<child-id>              select child configuration id
          -p, --pid-file=<pid-file>              write master process ID to the file specified
          -d, --debug-level=<debug-level>        set debugging level
          -c                <config>             supply configuration as a command line parameter
          --print                                print generated configuration to stdout
          --listen                               listen for clients...
             --listen_port=<nnn>                 use specified TCP port number
          --key=<secret_key>                     default clear-text secret key
          --host[=<name>]                        recognise a host
             --host_address=<cwaddr>[/<mask>]    IP address with optional net mask
             --host_key=<secret_key>             clear-text secret key
             --host_enable=[<level>]             define escalation to given level, 0..15 (default 15)
                --host_enable_password=<pass>    clear-text password
                --host_enable_permit             no password needed
                --host_enable_deny               escalation not allowed
          --group=<name>                         recognise a group
             --group_enable=[<level>]            define escalation to given level, 0..15 (default 15)
                --group_enable_password=<pass>   clear-text password
                --group_enable_permit            no password needed
                --group_enable_deny              escalation not allowed
          --user=<name>                          recognise a username
             --user_password=<pass>              clear-text password
             --user_permit                       user can log in with any password
             --user_deny                         user cannot log in with any password
             --user_default_cmd=deny             prohibit commands by default
             --user_default_cmd=permit           allow commands by default
             --user_cmd=<name>                   allow or prohibit use of specifed command
                --user_cmd_deny                  block all forms of this command
                --user_cmd_deny=<regex>          block matching forms of this command
                --user_cmd_permit                permit all forms of this command
                --user_cmd_permit=<regex>        permit matching forms of this command
             --user_group=<name>                 add user to the specified group
             --user_junos                        add junos-exec service stub


## Examples - CLI Configuration

      #!/tmp/tacacs/sbin/tac_plus

      id = spawnd {
          listen = { port = 4911 }
      }
      id = tac_plus {
          host = any { key = "some key" address = 0.0.0.0/0 }

          # repeat as necessary for each user
          user = fred {
              login = clear abcdef
              service = shell {
                  cmd = write  { permit terminal }
                  cmd = configure { permit .* }
              }
          }
      }

The equivalent command line version:

    #!/bin/bash

    exec /usr/local/sbin/tac_plus --print \
        --listen --listen_port=4911 \
        --host --host_key="some key" --host_address=0.0.0.0/0 \
        --user=fred --user_password=abcdef \
        --user_cmd=write --user_cmd_permit=terminal \
        --user_cmd=configure --user_cmd_permit
