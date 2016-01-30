# foo_xspf_1 [![AppVeyor Build](https://ci.appveyor.com/api/projects/status/github/Chocobo1/foo_xspf_1?branch=master&svg=true)](https://ci.appveyor.com/project/Chocobo1/foo-xspf-1)
XSPF playlist plugin for foobar2000 1.3+

XSPF is the XML format for sharing playlists.

### Features
* Generate XSPF playlist with/without hard coded path (\<location\>)
* Import XSPF playlist with/without hard coded path (\<location\>)
* Support XSPF version 1 format

### Download
[Project release page](https://github.com/Chocobo1/foo_xspf_1/releases)

### Usage
* To read a XSPF playlist, drag and drop the playlist to foobar2000, or just open it as you normally do (duh
* To generate a XSPF playlist, select menu `File` -> `Save playlist...`, or right click -> `Utilities` -> `Save as playlist...`

### Options
Options are listed at `File` -> `Preferences` -> page `Advanced` -> tab `Tools` -> `XSPF playlist`
  ![options_screenshot](https://raw.githubusercontent.com/Chocobo1/foo_xspf_1/master/pics/options.png)
* Read
  * \<album>

	Read this field and map to metadata "ALBUM". This field is used as text when \<location> is present, or as a search keyword when \<location> is absent.

  * \<creator>

    Read this field and map to metadata "ARTIST". This field is used as text when \<location> is present, or as a search keyword when \<location> is absent.

  * \<title>

    Read this field and map to metadata "TITLE". This field is used as text when \<location> is present, or as a search keyword when \<location> is absent.

  * \<trackNum>

    Read this field and map to metadata "TRACKNUMBER". This field is used as text when \<location> is present, or as a search keyword when \<location> is absent.

  * \<location>

    If checked, foobar2000 will use this field as file path. Otherwise the plugin will use other fields to find matching tracks in media library and add it to playlist.

  * Disable resolving \<location>

    If checked, the plugin won't bother to check if data in \<location> is valid or not. This option must be checked when \<location> is pointing to a cuesheet.

  * Allow multiple match

    If checked and \<location> is unchecked, the plugin will add all matching tracks instead of only one.

  * Allow partial matching

    If checked and \<location> is unchecked, the plugin will allow partial matching when finding tracks in media library. Otherwise, exact match is enforced.

* Write
  * \<album>

    Write out this field, mapped to metadata "ALBUM".

  * \<annotation>

    Write out this field, mapped to metadata "COMMENT".

  * \<creator>

    Write out this field, mapped to metadata "ARTIST".

  * \<date>

  Write out this field, mapped to metadata "TRACKNUMBER".

  * \<duration>

    Write out this field, mapped to the date on which the xspf file is generated.

  * \<location>

    Write out this field, mapped to "file path".

  * \<title>

    Write out this field, mapped to metadata "TITLE".

  * \<trackNum>

    Write out this field, mapped to metadata "TRACKNUMBER".

  * Compact output

    If checked, there won't be any formatting in the generated xspf file, making the file smaller but also makes it less suitable for human to read.

  * Use relative path whenever possible

    If checked, \<location> will have relative path instead of absolute path. The path is relative in the sense of where the track is located and the path of the generated xspf file.

### Compiler
* Visual Studio 2015

### License
See [LICENSE](https://github.com/Chocobo1/foo_xspf_1/blob/master/LICENSE) file

### Third-party code
* [TinyXML-2](http://www.grinninglizard.com/tinyxml2/)
* [foobar2000 Software Development Kit](https://www.foobar2000.org/SDK)

### References
* [XSPF](http://www.xspf.org/)
* [XSPF validator](http://validator.xspf.org/)
* [foobar2000](https://www.foobar2000.org/)
* [foobar2000 forum](https://hydrogenaud.io/index.php/board,28.0.html)
