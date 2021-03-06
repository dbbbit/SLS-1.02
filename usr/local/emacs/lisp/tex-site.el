;;;* Last edited: Jul 21 12:02 1992 (krab)
;;;;;;;;;;;;;;;;;;;;;;;;;;; -*- Mode: Emacs-Lisp -*- ;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 
;; tex-site.el - Site local variables for the auc-tex package
;; 
;; Copyright (C) 1991 Kresten Krab Thorup (krab@iesd.auc.dk).
;; 
;; 
;; 
;; $Id: tex-site.el,v 5.22 1992/09/16 12:14:22 amanda Exp $
;; 
;; LCD Archive Entry:
;; tex-site.el|Kresten Krab Thorup|krab@iesd.auc.dk
;; | Site local variables for the auc-tex package
;; |$Date: 1992/09/16 12:14:22 $|$Revision: 5.22 $|iesd.auc.dk:/pub/emacs-lisp
;; 
;; Author          : Kresten Krab Thorup
;; Created On      : Sun Sep  1 17:39:50 1991
;; Last Modified By: Per Abrahamsen
;; Last Modified On: Wed Sep  9 22:35:17 1992
;; Update Count    : 103
;; 
;; HISTORY (not updated)
;; 27-Jan-1992  (Last Mod: Mon Jan 27 15:49:26 1992 #64)  Kresten Krab Thorup
;;    Changed job-name-prefix to `_'... This will almost never
;;    conflict with anything... (hopefully)
;; 

(provide 'tex-site)

(defvar running-on-a-NeXT (if (getenv "HOSTTYPE")
			      (string-match "[Nn][Ee][Xx][Tt]"
					    (getenv "HOSTTYPE"))
			    nil)
  "*Special support for NeXT machines.")

(defvar TeX-print-command "dviprn -P%p %s"
  "*The command to print the .dvi file.

If the string contains the two letters `%p', it will be replaced with
the name of the selected printer.  Otherwise, the name of the printer
will be appended to the string, separated by \" -P\".

See TeX-default-printer-name and TeX-printer-name-alist for the
available printers.

If the string contains the two letters `%s', it will be replaced with
the name of the DVI file to be printed.  Otherwise, the name of
the file will simply be appended to the string, separated with a
space.

Thus \"dvips\" alone is equivalent to \"dvips -P%p %s\".")

(defvar TeX-printer-name-alist '(("lw1")
				 ("lw2")
				 ("lw4")
				 ("lj"))
  "*List of available printers.  
If this variable is nil, always use TeX-default-printer-name.")

(defvar TeX-default-printer-name "lj"
  "*Name of the printer used by the M-x TeX-print.")

(defvar TeX-default-jobname-prefix "_"
  "*Prefix to set on temporary files when running TeX-region.")

(defvar TeX-args nil
  "*A LIST of arguments to supply to LaTeX/TeX. 
If you're using a NeXT you could e.g. say

   (setq TeX-args '(\"-v\"))

in your .emacs file, to have LaTeX preview as it runs.

OBSOLETE: Please use plain-TeX-command or LaTeX-command instead.
This variable will not be supported in a future release.")

(defvar plain-TeX-command
  ; Horrible kludge to support TeX-args. Remove it in v. 7.0!
  (let ((result "tex")
	(rest TeX-args))
    (while rest
      (setq result (concat result " " (car rest)))
      (setq rest (cdr rest)))
    result)
  "*The command used for formatting plain TeX documents.
If the string contains the two letters `%s', it will be replaced with
the name of the plain TeX file to be formatted.  Otherwise, the name
of the file will simply be appended to the string, separated with a
space.") 
  
(defvar LaTeX-command
  ; Horrible kludge to support TeX-args. Remove it in v. 7.0!
  (let ((result "latex")
	(rest TeX-args))
    (while rest
      (setq result (concat result " " (car rest)))
      (setq rest (cdr rest)))
    result)
  "*The command used for formatting LaTeX documents.
If the string contains the two letters `%s', it will be replaced with
the name of the LaTeX file to be formatted.  Otherwise, the name of
the file will simply be appended to the string, separated with a
space.") 

(defvar TeX-bibtex-command "bibtex"
  "*The command to run BibTeX")

(defvar TeX-index-command "makeindex"
  "*The command to run a program generating an index, usually \"makeindex\"")

(defvar TeX-index-suffix ".ind"
  "*The suffix (file-name-extention) of the file generated by \\[LaTeX-makeindex]")

(defvar TeX-compilation-cd-command "cd"
  "*Command to give to shell running TeX to change directory.  The expanded
value of TeX-directory will be appended to this, separated by a space.")

(defvar TeX-default-mode 'plain-tex-mode
  "*Mode to enter for a new file when it can't be determined whether
the file is plain TeX or LaTeX or what.")

(defvar TeX-force-default-mode nil
  "*If set to nil, try to infer the mode of the file from its content.")

(defvar LaTeX-default-environment "itemize"
  "*The default environment when creating new ones with
LaTeX-environment.")

(defvar LaTeX-default-style "article"
  "*Default when creating new documents.")

(defvar LaTeX-default-style-options ""
  "*Default options to documentstyle")

(defvar LaTeX-float "tbp"
  "*Default float when creating figure and table environments.")

(defvar LaTeX-figure-label "fig:"
  "*Default prefix to figure labels.")

(defvar LaTeX-table-label "tab:"
  "*Default prefix to table labels.")

(defvar LaTeX-section-label "sec:"
  "*Default prefix when asking for a label.")

;; one of `LATEX', `OLATEX' `TEX' or `SLITEX'... `LATEX' uses the new
;; font selection scheme, while `OLATEX' uses the old.

(defvar LaTeX-format-package '( "LATEX" ))

(defvar TeX-format-package '( "TEX" ))

;;
;;

(defvar TeX-preview-command nil
  "*The command used for previewing.  If the string contains the two
letters `%s', it will be replaced with the name of the DVI file
to be formatted.  Otherwise, the name of the file will simply be
appended to the string, separated with a space.

OBSOLETE.  Will be removed in a future release.

Only used when no match is found with TeX-preview-alist.")

(defvar TeX-complete-word (key-binding "\e\t")
  "*Function to call if M-t is invoked on a word which does not start 
with a backslash. Default is the meaning of M-t when latex-mode was
first invoked.")

(defvar LaTeX-indent-level 2
  "*Indentation of begin-end blocks in LaTeX.")

(defvar LaTeX-item-indent -2
  "*Extra indentation for lines beginning with \\item{.")

(defvar TeX-auto-header "texheader"
  "*filename for TeX-headers. \input by TeX, so the real file should have
the extention `.tex'")

(defvar TeX-auto-trailer "textrailer"
  "*filename for TeX-trailers \input by TeX, so the real file should have
the extention `.tex'")


(defvar TeX-preview-landscape
  (cond
   ;; if we're in OpenWindows2
   ((getenv "NEWSSERVER")	
    "dvips -Pageview")
   
   ;; if we're in OpenWindows3
   ((string= (getenv "WINDOW_SYSTEM") "OW3")
    "dvips -Pageview")
   
   ;; if on NeXT
   (running-on-a-NeXT
    "open")
   
   ;; else if in the M.I.T. X window system
   ((getenv "DISPLAY")
    "xdvi -paper usr -s 4")
   
   ;; else we assume some terminal
   (t "dvitty -q -w 160"))
  "Command used for landscape preview.  If the string contains the two
letters `%s', it will be replaced with the name of the DVI file
to be formatted.  Otherwise, the name of the file will simply be
appended to the string, separated with a space.")

(defvar TeX-preview-a5
  (cond
   ;; if we're in OpenWindows2
   ((getenv "NEWSSERVER")	
    "dvips -Pageview")
   
   ;; if we're in OpenWindows3
   ((string= (getenv "WINDOW_SYSTEM") "OW3")
    "dvips -Pageview")
   
   ;; else if in the M.I.T. X window system
   ((getenv "DISPLAY")
    "xdvi -paper a5")
   
   ;; if on NeXT
   (running-on-a-NeXT
    "open")
   
   ;; else we assume some terminal
   (t "dvitty -q -w 80"))
  "Command used for a5 preview.  If the string contains the two
letters `%s', it will be replaced with the name of the DVI file
to be formatted.  Otherwise, the name of the file will simply be
appended to the string, separated with a space.")

(defvar TeX-preview-portrait
  (cond
   ;; if we're in OpenWindows2
   ((getenv "NEWSSERVER")	
    "dvips -Pageview")
   
   ;; if we're in OpenWindows3
   ((string= (getenv "WINDOW_SYSTEM") "OW3")
    "dvips -Pageview")
   
   ;; if on NeXT
   (running-on-a-NeXT
    "open")
   
   ;; else if in the M.I.T. X window system
   ((getenv "DISPLAY")
    "xdvi -s 2")
   
   ;; else we assume some terminal
   (t "dvitty -q -w 80"))
  "Command used for potrait preview.  If the string contains the two
letters `%s', it will be replaced with the name of the DVI file
to be formatted.  Otherwise, the name of the file will simply be
appended to the string, separated with a space.")

(defvar TeX-preview-alist
  '(("\\\\documentstyle.*landscape" . TeX-preview-landscape )
    ("\\\\documentstyle.*a5" . TeX-preview-a5 )
    ("." . TeX-preview-portrait ))
  
  "*An alist where the key is a regexp, and the value is the
formatting command to be invoked if the regexp is found in the TeX
file. 

If the value string contains the two letters `%s', it will be replaced
with the name of the DVI file to be previewed.  Otherwise, the name of
the file will simply be appended to the string, separated with a
space.")

(defvar TeX-smart-quotes t
  "*If set to t, remap \" to insert either `` before a word, or ''
after a word.  Users of german.sty, or other people who uses the 
literal \" often, will want to disable it by setting this variable to
nil.")

