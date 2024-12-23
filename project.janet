(declare-project
  :name "timing"
  :description "Benchmarking macro for Janet"
  :author "jsks"
  :license "MIT"
  :url "https://github.com/jsks/janet-timing"
  :repo "git+https://github.com/jsks/janet-timing")

(declare-native
  :name "timing/native"
  :source ["clocktime.c"])

(declare-source
  :source "timing.janet")
