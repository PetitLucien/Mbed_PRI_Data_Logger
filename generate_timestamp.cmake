# Génère un timestamp Unix (UTC)
string(TIMESTAMP UNIX_TIMESTAMP "%s" UTC)

file(WRITE ${OUTPUT}
"#pragma once\n#define TIMESTAMP ${UNIX_TIMESTAMP}\n")
