hash = %x[git rev-parse HEAD]
command = "sed -i 's/#define PROVLIB_COMMIT.*/#define PROVLIB_COMMIT \""+hash.chop!+"\"/' ./include/provenance.h"
puts command
exec(command)
