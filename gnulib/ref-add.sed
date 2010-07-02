/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ sem / sem /
  tb
  s/ $/ sem /
  :b
  s/^/# Packages using this file:/
}
