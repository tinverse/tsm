                                        ; -*- mode: Lisp -*-
(setq cmake-ide-build-dir '"build/")

(c++-mode . ((helm-make-build-dir . "build/")
             (helm-make-arguments . "-j4")
              (indent-tabs-mode . nil)
              (tab-width . 4)
              (c-basic-offset . 'tab-width)))

(cmake-mode . ((cmake-ide-project-dir . "./")
         (cmake-ide-build-dir . "build/")
         (cmake-ide-cmake-opts . "-DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_DEPENDENCIES=OFF")))

(nil . ((projectile-project-name . "tsm")
                    (projectile-project-run-cmd . "ctest")
                    (projectile-project-test-cmd . "ctest")))

(nil . ((helm-ctest-build-dir . "build/")
        (helm-make-build-dir . "build/")
        (helm-make-arguments . "-j4")))

(nil . ((desktop-save-mode 1)))
