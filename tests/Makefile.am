
if LANG_FRONTEND

Lang = parsing

endif

if LANG_RUNTIME

RunTime = basic sw eav ecu

endif

if CompileOk

Compiler = cc

endif

SUBDIRS = $(Lang) $(RunTime) lemon $(Compiler)

EXTRA_DIST = rdriver.pm regress_utils.pm
