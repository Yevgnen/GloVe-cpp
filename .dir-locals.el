;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((c++-mode
  (projectile-project-compilation-cmd . "cd build && cmake .. -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=Release && cmake --build . -- -j 6")))

