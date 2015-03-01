;;;; app-view.lisp

(in-package #:monster-avengers.simple-web)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (enable-jsx-reader))

(def-widget title-bar (language callback)
    ()
  #jsx(:div ((class-name "topbar")
             (style :width "auto"
                    :margin "auto"))
            (:div ((class-name "fill row"))
                  (:div ((class-name "col-md-6"))
                        (:div ((class-name "page-header"))
                              (:h1 () (lang-text ("en" "Monster Hunter 4 Ultimate Armor Set Search")
                                                 ("zh" "怪物猎人4G 配装器")))))
                  (:div ((class-name "col-md-2 col-md-offset-4"))
                        (:select ((class-name "form-control")
                                  (value language)
                                  (on-change (lambda (e)
                                               (funcall callback
                                                        (@ e target value)))))
                                 (:option ((value "en")) "English (英文)")
                                 (:option ((value "zh")) "中文 (Chinese)"))))))

(def-widget tab-panel (language pages callback active-page)
    ()
  #jsx(:div ((style :margin-bottom "20px"))
            (:ul ((class-name "nav nav-pills"))
                 (chain pages 
                        (map (lambda (page)
                               (:li ((class-name 
                                      (if (= (@ page name) active-page) 
                                          "active" "")))
                                    (:a ((href "#")
                                         (on-click (lambda () 
                                                     (funcall callback (@ page name)))))
                                        (lang-text ("en" (@ page en))
                                                   ("zh" (@ page zh)))))))))))


(def-widget info-panel (language)
    ()
  #jsx(:div ()
            (:p () "Built by "
                (:a ((href "https://github.com/breakds")) "BreakDS")
                ", with "
                (:a ((href "https://github.com/breakds/realispic")) "reaLISPic")
                ", "
                (:a ((href "http://www.sbcl.org")) "SBCL")
                " and "
                (:a ((href "http://getbootstrap.com/")) "Twitter Bootstrap")
                ".")
            (:p () "Credit to the project "
                (:a ((href "https://github.com/kamegami13/MonsterHunter4UDatabase"))
                    "MonsterHunter4UDatabase")
                (+ " for armor, skills and jewels data. "
                   "Without their effort I could not have accomplished this."))
            (:p () "Deisgn and artwork by Cassandra Qi.")))

(def-widget app-view (default-language)
    ((state (language (if default-language
                          default-language
                          "zh"))
            (weapon-type "melee")
            (weapon-holes 0)
            (rare 1)
            (current-page "search")
            (amulets (array))
            (effects (array))
            (blacklist (array))
            (query-fail false)
            (query-result (array))
            (in-progress false))
     (switch-language (target) 
                      (chain this (set-state (create language target))))
     (append-blacklist (id)
                       (let ((new-blacklist (local-state blacklist)))
                         (chain new-blacklist (push id))
                         (chain this (set-state (create blacklist
                                                        new-blacklist))))
                       nil)
     (handle-query (is-filter)
                   (let ((query ""))
                     (setf query (+ query "(:weapon-type \""
                                    (local-state weapon-type) "\") "))
                     (setf query (+ query "(:weapon-holes "
                                    (local-state weapon-holes) ") "))
                     (setf query (+ query "(:rare "
                                    (local-state rare) ") "))
                     (loop for effect in (local-state effects)
                        do (setf query (+ query "(:skill "
                                          (@ effect id) " "
                                          (@ effect points) ") ")))
                     (loop for amulet in (local-state amulets)
                        do (progn (setf query (+ query "(:amulet " 
                                                 (aref amulet 0)
                                                 " ("))
                                  (when (> (@ amulet length) 1)
                                    (setf query (+ query (aref amulet 1)))
                                    (loop for i from 2 below (@ amulet length)
                                       do (setf query (+ query " "
                                                         (aref amulet i)))))
                                  (setf query (+ query ")) "))))
                     ;; blacklist
                     (when (not is-filter)
                       (chain this (set-state (create blacklist (array)))))
                     (when (and (> (@ (local-state blacklist) length) 0)
                                is-filter)
                       (setf query (+ query "(:blacklist ("
                                      (aref (local-state blacklist) 0)))
                       (loop for i from 1 
                          below (@ (local-state blacklist) length)
                          do (setf query (+ query " " 
                                            (aref (local-state blacklist) i))))
                       (setf query (+ query ")) ")))
                     (chain console (log query))
                     (chain this (set-state (create in-progress true
                                                    query-result (array))))
                     (with-rpc (answer-query query)
                       (chain this (set-state (create query-result rpc-result
                                                      in-progress false
                                                      query-fail (<= (@ rpc-result length) 0))))
                       (funcall (@ this switch-page) "result")
                       (chain console (log rpc-result))))
                   nil)
     (update-parameters (param value)
                        (if (= param "weapon-type")
                            (chain this (set-state (create weapon-type value)))
                            (if (= param "weapon-holes")
                                (chain this (set-state (create weapon-holes value)))
                                (if (= param "rare")
                                    (chain this (set-state (create rare value))))))
                        nil)
     (update-amulets (new-amulets)
                     (chain this (set-state (create amulets new-amulets)))
                     nil)
     (update-effects (skill-id active-id)
		     (let ((new-effects (local-state effects)))
		       (setf new-effects
			     (chain new-effects (filter (lambda (e i a) 
							  (!= (@ e id)
							      skill-id)))))
		       (when (> active-id -1)
			 (let ((points (@ (aref (@ (aref skill-systems skill-id) skills)
						active-id) points)))
			   (chain new-effects (push (create :id skill-id
                                                            :active active-id
							    :points points)))))
		       (chain this (set-state (create effects new-effects))))
		     nil)
     (switch-page (page)
                  (chain this (set-state (create current-page page)))
                  nil))
  #jsx(:div ((style :margin "20px 50px 30px 50px"))
            (:title-bar ((language (@ this state language))
                         (callback (@ this switch-language))))
            (:tab-panel ((pages (array (create name "search" 
                                               en "Search"
                                               zh "搜索页面")
                                       (create name "result"
                                               en "Result"
                                               zh "配装结果")
                                       (create name "help"
                                               en "Help"
                                               zh "帮助")))
                         (language (local-state language))
                         (callback (@ this switch-page))
                         (active-page (local-state current-page))))
            (if (= (local-state current-page) "search")
                (:div ()
                      (:parameter-panel ((language (@ this state language))
                                         (weapon-type (local-state weapon-type))
                                         (weapon-holes (local-state weapon-holes))
                                         (rare (local-state rare))
                                         (callback (@ this update-parameters))))
                      (:div ((class-name "row"))
                            (:div ((class-name "col-md-3"))
                                  (:amulet-panel ((:language (local-state language))
                                                  (:amulets (local-state amulets))
                                                  (:callback (@ this update-amulets))))
                                  (:info-panel ((:language (local-state language)))))
                            (:div ((class-name "col-md-3"))
                                  (:skill-panel ((language (local-state language))
                                                 (effects (local-state effects))
                                                 (change-callback (@ this update-effects))))
                                  (:button ((class-name "btn btn-primary")
                                            (disabled (local-state in-progress))
                                            (on-click (lambda () 
                                                        (funcall (@ this handle-query) false)
                                                        nil)))
                                           (if (local-state in-progress)
                                               (if (= (local-state "language") "en")
                                                   "Working..."
                                                   "执行中...")
                                               (if (= (local-state "language") "en")
                                                   "Search"
                                                   "搜索"))))))
                (if (= (local-state current-page) "result")
                    (:div ((class-name "row"))
                          (:div ((class-name "col-md-6"))
                                (cond ((local-state query-fail)
                                       (:query-fail-alert ((:language (local-state language)))))
                                      ((local-state in-progress)
                                       (:in-progress-alert ((:language (local-state language)))))
                                      (t (chain (local-state query-result)
                                                (map (lambda (armor-set)
                                                       (:armor-set-display 
                                                        ((language (local-state language))
                                                         (weapon (local-state weapon-type))
                                                         (blacklist-callback (@ this append-blacklist))
                                                         (filter-callback (@ this handle-query))
                                                         (armor-set armor-set))))))))))
                    (:div ((class-name "row"))
                          (:div ((class-name "col-md-6"))
                                (:help-panel ((language (local-state language))))))))))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (disable-jsx-reader))