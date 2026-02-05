
target_sources(app PRIVATE i3c_mchp.c)
target_sources(app PRIVATE himax.c)
target_sources(app PRIVATE btn.c)
target_sources(app PRIVATE fan.c)

target_sources_ifdef(CONFIG_PM app PRIVATE power.c)
