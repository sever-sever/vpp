.. _settingupenvironment:

Setting up your environment
===========================

All of these exercises are designed to be performed on an Ubuntu 22.04 (Jammy) box.

* If you have an Ubuntu 22.04 box on which you have sudo or root access, you can feel free to use that.
* If you do not, a Vagrantfile is provided to setup a basic Ubuntu 22.04 box for you in the the steps below.

Install Libvirt and Vagrant
-------------------------------

You will need to install Libvirt and Vagrant.

Create a Vagrant Directory
---------------------------

To get started create a directory for vagrant

.. code-block:: console

   $ mkdir vpp-tutorial
   $ cd vpp-tutorial

Create a file called **Vagrantfile** with the following contents:

.. code-block:: ruby

    # -*- mode: ruby -*-
    # vi: set ft=ruby :

    Vagrant.configure(2) do |config|

      config.vm.box = "generic/ubuntu2204"

      vmcpu=(ENV['VPP_VAGRANT_VMCPU'] || 2)
      vmram=(ENV['VPP_VAGRANT_VMRAM'] || 4096)

      config.ssh.forward_agent = true

      config.vm.provider "libvirt" do |vb|
          vb.memory = "#{vmram}"
          vb.cpus = "#{vmcpu}"
      end
    end


Running Vagrant
---------------

VPP runs in userspace.  In a production environment you will often run it with
DPDK to connect to real NICs or vhost to connect to VMs.mIn those circumstances
you usually run a single instance of VPP.

For purposes of this tutorial, it is going to be extremely useful to run multiple
instances of vpp, and connect them to each other to form a topology.  Fortunately,
VPP supports this.

When running multiple VPP instances, each instance needs to have specified a 'name'
or 'prefix'.  In the example below, the 'name' or 'prefix' is "vpp1". Note that only
one instance can use the dpdk plugin, since this plugin is trying to acquire a lock
on a file.

Setting up VPP environment with Vagrant
---------------------------------------------

After setting up Vagrant, use these commands on your Vagrant directory to boot the VM:

.. code-block:: console

    $ vagrant up
    $ vagrant ssh
    $ sudo bash
    # apt-get update
    # reboot -n
    $ # Wait for the VM to reboot
    $ vagrant ssh

Install VPP
------------

Now that the VM is updated, we will install the VPP packages.

For more on installing VPP please refer to :ref:`installingVPP`.

For this tutorial we will install VPP by modifying the file
**/etc/apt/sources.list.d/99fd.io.list**.

We write this file with the following contents:

.. code-block:: console

   $ sudo bash
   # echo "deb https://packagecloud.io/fdio/release/ubuntu jammy main" > /etc/apt/sources.list.d/99fd.io.list
   #

Get the key.

.. code-block:: console

   # curl -L https://packagecloud.io/fdio/release/gpgkey | sudo apt-key add -
   #

Then execute the following commands.

.. code-block:: console

   # apt-get update
   # apt-get install vpp vpp-plugin-core vpp-plugin-dpdk
   #

Stop VPP for this tutorial. We will be creating our own instances of VPP.

.. code-block:: console

   # service vpp stop
   #


Create some startup files
--------------------------

We will create some startup files for the use of this tutorial. Typically you will
modify the startup.conf file found in /etc/vpp/startup.conf. For more information
on this file refer to :ref:`configuration_reference`.

When running multiple VPP instances, each instance needs to have
specified a 'name' or 'prefix'. In the example below, the 'name' or 'prefix'
is "vpp1". Note that only one instance can use the dpdk plugin, since this
plugin is trying to acquire a lock on a file. These startup files we create will
disable the dpdk plugin.

Also in our startup files notice **api-segment**. **api-segment {prefix vpp1}**
tells FD.io VPP how to name the files in /dev/shm/ for your VPP instance
differently from the default. **unix {cli-listen /run/vpp/cli-vpp1.sock}**
tells vpp to use a non-default socket file when being addressed by vppctl.

Now create 2 files named startup1.conf and startup2.conf with the following
content. These files can be located anywhere. We specify the location when we
start VPP.

startup1.conf:

.. code-block:: console

   unix {cli-listen /run/vpp/cli-vpp1.sock}
   api-segment { prefix vpp1 }
   plugins { plugin dpdk_plugin.so { disable } }

startup2.conf:

.. code-block:: console

   unix {cli-listen /run/vpp/cli-vpp2.sock}
   api-segment { prefix vpp2 }
   plugins { plugin dpdk_plugin.so { disable } }
