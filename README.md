# Danger Mushroom
A fork of cicuta_virosa that working on ios 16.1.2 by Seal4ever.

# Current state
- Exploit works only on A13+ devices :(
- need a lot of time to start working
- if your device is on a A15 you will get kernel panic
- not finished

# How to build it
We don't like to commit Xcode project file. Create your own XCode project, add files and call "danger_mushroom" function.

# Credits
- Some utils (exploit_utilities.c): @Jakeashacks
- Vuln: Apple  

# PAC bypass
- the pac bypass will not work on ios 16.1.1 because exploit couldn't acces
the kernel and you will get panic
- on ios 16.0 beta 1 and ios 16.1.2 pac bypass working succefuly

