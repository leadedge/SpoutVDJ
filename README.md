# SpoutVDJ
Spout plugins for [Virtual DJ](https://www.virtualdj.com/) developed using the VirtualDJ 8 plugin SDK and Spout SDK version 2.007.013.

There are two plugin projects, VDJSpoutReceiver64 and VDJSpoutSender64, both 64 bit only. The plugins will not work on VirtualDJ 7 or earlier.

## Building the projects

  1) Create or use any convenient folder : e.g. "myProjects"
  2) Download the ["beta"](https://github.com/leadedge/Spout2/tree/beta) branch of the Spout SDK
  3) Copy the "SpoutGL" folder into "myProjects"
  4) Create another subfolder e.g. myApps
  5) Download this repository
  6) Copy the "SpoutSender64" and "SpoutReceiver64" folders and contents to "myProjects\myApps".
  
You will then have a folder structure as follows :
  
&nbsp;&nbsp;&nbsp;&nbsp;myProjects\
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;SpoutGL <- the Spout SDK source files\
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;myApps\
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;SpoutSender64 <- the sender plugin project folder\
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;SpoutReceiver64 <- the receiver plugin project folder\
                 
Open each solution file with Visual Studio 2022 and build "Release".

See Releases for binaries.  

VDJSpoutSender64.dll must be copied to - Documents > VirtualDJ > Plugins64 > VideoEffect

VDJSpoutReceiver64.dll must be copied to - Documents > VirtualDJ > Plugins64 > Visualisations

"SpoutPanel.exe" is used by VDJSpoutReceiver64 to show a list of Spout senders and will be detected after SpoutSettings from the latest [Spout release](https://leadedge.github.io/spout-download.html) has been run once.

Your support will help the project to continue and develop.\
Become a [GitHub Sponsor](https://github.com/sponsors/leadedge) and show your support.\
Or make a donation :\
[![](https://www.paypalobjects.com/en_AU/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=P4P4QJZBT87PJ)  


