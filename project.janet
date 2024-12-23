(declare-project
  :name "timing"
  :license "MIT"
  :repo "git+https://github.com/jsks/janet-timing")

(declare-native
  :name "timing/native"
  :source ["clocktime.c"])

(declare-source
  :source "timing.janet"
  :prefix "timing")
