# casperscript - ghostscript/postscript made friendlier for Python and C programmers

The goal is to make casperscript an extension of ghostscript optimized for
general-purpose programming. I don't envision it used for embedded devices,
although I won't deliberately code the extensions so as to prohibit such.

One of the first steps was to define '#!' as a comment, obviating having to
use `binfmt_misc` and a wrapper script for the interpreter (because to
process args, gs must be run as `gs -- myfile.ps arg1 arg2`).

This also required splitting the args, since linux returns multiple
shebang-line args as a single string.

A change that turned out to be far simpler than expected got rid of the
/undefined error, enabling easy top-down design: a placeholder is shown
for any undefined words at time of command execution.

Now the primary goal is to give it the "batteries included" features from
Python, so budding programmers don't have to "reinvent the wheel" just to
perform simple tasks.

I was hoping to be able to hijack the entire parser so that '/' could be
used for division and '#' for comments, on a per-file basis (so real
postscript code can still be run), but that's going to take a back seat.
Too complicated.

John Comeau jc@unternet.net 2023-03-21

# links
[Information for Ghostscript developers](https://ghostscript.com/docs/9.54.0/Develop.htm)

# quickstart

Go to digitalocean.com, set up an account if you don't have one already, and
spin up a $6/month Debian 11 droplet, the most basic configuration. It's less
than a penny an hour, and you only pay for the time you use.

`ssh root@droplet`, where `droplet` is the IP number assigned to your Debian
instance by digitalocean.

```bash
apt update
apt install git make autoconf gcc libreadline-dev libx11-dev x11-apps xauth libxext-dev
echo 'export PATH=$HOME/bin:$PATH' >> ~/.bashrc
^D  # logout, we'll log right back in with an X windows connection
ssh -Y root@droplet
cd /usr/src
git clone https://github.com/jcomeauictx/casperscript
cd casperscript
make
make install
csbin/echo just a test  # should echo 'just a test' on the following line
ccs  # should give you a `ccs>` prompt
ccs> (hello world!) =
hello world!
^D  # log out of ccs
xeyes  # make sure you have a working X connection
cs examples/tiger.eps  # should show a beautiful color cartoon tiger
```

Don't forget to go back to digitalocean.com and delete your droplet!
