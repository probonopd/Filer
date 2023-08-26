#include "AppGlobals.h"
#include "VolumeWatcher.h"

// This is just an example of how to use inline namespaces.
// To use it:
// QColor bgColor = AppGlobals::Colors::BackgroundColor;
const QColor AppGlobals::Colors::BackgroundColor(255, 255, 255);
const QColor AppGlobals::Colors::TextColor(0, 0, 0);

// This is just an example.
// To use it:
// int maxItems = AppGlobals::MaxItems;
const int AppGlobals::MaxItems = 100;

// Global application-wide variable that holds /media or /media/$USER
const QString AppGlobals::mediaPath = VolumeWatcher::getMediaPath();

// TODO: Actually read the name of the start volume from the volume label
const QString AppGlobals::hardDiskName = "Hard Disk";

const QString AppGlobals::desktopPicturePath = "/usr/local/share/slim/themes/default/background.jpg";