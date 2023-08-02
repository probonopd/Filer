# Filer [![Build Status](https://api.cirrus-ci.com/probonopd/helloSystem/Filer.svg)](https://cirrus-ci.com/github/probonopd/Filer) [![Ubuntu Build Status](https://img.shields.io/github/actions/workflow/status/probonopd/Filer/ubuntu.yml?branch=main)](https://github.com/probonopd/Filer/actions/workflows/ubuntu.yml)

A clean rewrite of the Filer for [helloSystem](https://hellosystem.github.io/) (but not limited to it), inspired by [John Siracusa's descriptions of "Spatial Orientation"](https://arstechnica.com/gadgets/2003/04/finder/).

![](https://user-images.githubusercontent.com/2480569/206650534-a035fc64-5993-41dd-8753-1076f47598c5.png)

* No GLib dependency
* No libfm dependency
* Minimal dependencies other than Qt
* No inherited code
* Uses [launch](https://github.com/helloSystem/launch/) "database"
* Runs on FreeBSD (developed for [helloSystem](https://hellosystem.github.io/)) and on Linux (occasionally tested on Lubutnu)

## Features

|Other file managers|Filer|Implemented|
|---|---|---|
|Do not natively support `.app` bundles, `.AppDir` and `.AppImage` formats | Natively supports `.app` bundles, `.AppDir` and `.AppImage` formats|WIP|
|Use XDG standards that have prevented *nix desktops from working well for all too long|Engineered from first principles to be a great desktop file manager|WIP|
|Assume applications are at fixed locations, e.g., in `/usr/bin`|Assumes applications can be anywhere including external disks and file shares, and can be freely moved around|WIP based on `launch` "database"|
|Show icons for files based on their MIME type|Show icons for files based on the application that opens them|Implemented based on `launch` "database"|
|Do not allow individual files of the same MIME type to be opened with different applications|Allows individual files of the same MIME type to be opened with different applications (using the `can-open` extended attribute)|Handled by `launch`|
|Do not persist window location and size for each window|Persists window location and size for each window|Implemented|
|Do not persist the view type for each window|Persists the view type for each window|Implemented|
|Do not persist icon locations|Persists icon locations|TODO|
|Do not persist the size of each icon|Persists the size of each icon|For consideration|
|Cannot use copy and pasted icons for documents and folders|Can use copy and pasted icons for documents and folders|TODO|
|Open files with whatever application if there is more than one that can handle a file|Asks the user which application to use if there is more than one that can handle a file|Handled by `launch`|
|Do not know which application created a file|Knows which application created a file and by default opens it with that application|TODO; how? Using DTrace or similar?|
|Do not allow drag-and-drop of files onto applications|Allows drag-and-drop of files onto applications|TODO|
|No animation when applications, documents, or folders are opened|Animation when applications, documents, or folders are opened|Implemented (using QTimeLine Property Animation)|
|Opened applications, documents, and folders are not shown as such|Opened applications, documents, and folders are shown as such|Implemented for folders, TODO for the other types|

My conclusion is that most "desktop environments" are made by people who don't actually love the desktop. Or else, those basic things would work.

## References

* https://arstechnica.com/gadgets/2003/04/finder/

* https://daringfireball.net/2003/04/siracusa_on_the_finder

* https://www.asktog.com/columns/038MacUITrends.html

* https://apple.stackexchange.com/questions/106681/is-there-software-that-implements-john-siracusas-spatial-finder-concept

* https://dfeldman.medium.com/about-the-spatial-debate-4ccb8064f1df

* https://news.ycombinator.com/item?id=12484241
