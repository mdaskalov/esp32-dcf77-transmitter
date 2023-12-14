Import('env')

import os
import shutil

# copy defaults_override_sample.ini to defaults_override.ini
if os.path.isfile("defaults_override.ini"):
    print ("Using provided defaults_override.ini")
else:
    shutil.copy("defaults_override_sample.ini", "defaults_override.ini")

