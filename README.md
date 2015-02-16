# foo_xspf_1
XSPF playlist plugin for foobar2000

Support XSPF version 1 format

### Usage
* To read a XSPF playlist, drag and drop the playlist to foobar2000, or just open it as you normally do (duh
* To generate a XSPF playlist, select menu `File` -> `Save playlist...`, or right click -> `Utilities` -> `Save as playlist...`

### Compile
* Visual Studio 2013 Express
* Turn off [Link-time Code Generation (LTCG)](https://msdn.microsoft.com/en-us/library/xbf3tbeh.aspx) in `foobar2000_SDK` project. Switching it on gives me bad allocation error when getting info from metaDB

### License
See [LICENSE](https://github.com/Chocobo1/foo_xspf_1/blob/master/LICENSE) file

### Third-party code
* [TinyXML-2](http://www.grinninglizard.com/tinyxml2/)
* [foobar2000 Software Development Kit](http://www.foobar2000.org/SDK)

### References
* [XSPF](http://www.xspf.org/)
* [XSPF validator](http://validator.xspf.org/)
* [foobar2000](http://www.foobar2000.org/)
* [foobar2000 forum](http://www.hydrogenaud.io/forums/index.php?act=SF&s=&f=28)
