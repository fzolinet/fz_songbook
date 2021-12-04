The fz_songbook a text filter.
the idea came from Linkesoft Songbook https://www.linkesoft.com/songbook/

You can use in two mode
1. [songbook src="path-to-file/filename.pro" /]
The filter read the songbook file and shows it just like a songbook in the body of node.

2. [songbook] songbook text... [/songbook]
The songbook text shows the module like a songbook text...

The LinkeSoft songbook: https://www.linkesoft.com/songbook/

The songbook has a header. In the header you can use tags:
{title:....} or {t:....} - the title of song
{suntitle: ...} or {st: ...} - the subtitle of song, for example the name of composer, lyricist
{time:...} - the time of song
{key:...}  - the key of the song
{tag:....} - the tag can be anything, for example rock, Something Blues Band...
{musicpath:.....} - the place of sound material. It can:
 - http:// https:// ftp:// - from the internet, 
   for example https://www.youtube.com/watch?v=WHtWs4wiFCs.
 - public:// - from Drupal filesystem public folder
 - file:// - From settings of fz_songbook,  tipically public://
 - something /folder/filename.mp3 - From root of Drupal 

 If Smplphotoalbum installed then the path is a subfolder of smplphotoalbum root folder.
 - /sites/all/modules/smplphotoalbum/v.php?i=/Folder/filename.mp3 
   will shows only the filename, but the link helps the play this song. 
 - /folder/filename.mp3 - shows onlíy fkilename but the link help to play the song with 
   smplphotoalbum
