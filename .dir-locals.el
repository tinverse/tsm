(use-package rtags
  :config
  (rtags-enable-standard-keybindings)
  (setq rtags-autostart-diagnostics t)
  (rtags-diagnostics)
  (setq rtags-completions-enabled t)
  (rtags-start-process-unless-running))
(use-package cmake-ide
  :config
  (cmake-ide-setup))
(setq cmake-ide-build-dir '"build/")

((c++-mode (helm-make-build-dir . "build/")))
