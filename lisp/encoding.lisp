;;;; encoding.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;; 
;;;; Description: Component of the package armor-up that provides
;;;; subroutines to encode/decode skill signatures and hole signatures

(in-package #:breakds.monster-avengers.armor-up)



;;; ---------- Hole Signatures ----------
;;; A hole signature is a 3-length list such as (2 3 1), where
;;; first element -> number of 1-holes
;;; second element -> number of 2-holes
;;; third element -> number of 3-holes

(declaim (inline encode-hole-sig))
(defun encode-hole-sig (hole-sig)
  "This function encode a hole signature into a 64-bit unsigned
  integer. Out of the 64 bits, the first 12 bits are used to encode
  the hole signature, where the number of each type of hole have 4
  bits for itself."
  (the (unsigned-byte 64)
       (logior (ldb (byte 4 0) 
                    (the (signed-byte 8) (nth 0 hole-sig)))
               (ash (ldb (byte 4 0) 
                         (the (signed-byte 8) (nth 1 hole-sig))) 4)
               (ash (ldb (byte 4 0) 
                         (the (signed-byte 8) (nth 2 hole-sig))) 8))))

(declaim (inline decode-hole-sig))
(defun decode-hole-sig (key)
  "The inverse function of encode-hole-sig."
  (declare (type (unsigned-byte 64) key))
  (list (logand key #b1111)
        (logand (ash key -4) #b1111)
        (logand (ash key -8) #b1111)))



;;; ---------- Skill Signatures ----------
;;; A skill signature is an arbitrary long list such as (5 -3) and (-1
;;; -2 6). Each number stands for the total points of a skill-system.

(declaim (inline encode-skill-sig))
(defun encode-skill-sig (skill-sig)
  "This function encode a skill signature into a 64-bit unsigned
  integer. Out of the 64 bits, the last 52 bits are used to encode the
  hole signature, where the points of each skill gets 8 bit in order."
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for points in skill-sig
       for offset from 12 by 6
       do (setf result 
                (logior result
                        (ash (ldb (byte 6 0) points) offset))))
    result))

(declaim (inline decode-skill-sig))
(defun decode-skill-sig (key n)
  "The semi inverse function of encode-skill-sig. This is different
  from an actual inverse function of encode-skill-sig as it represent
  negative number as its complement code."
  (declare (type (unsigned-byte 64) key))
  (loop
     for i below n
     for offset from 12 by 6
     collect (logand (ash key (- offset)) #b111111)))

(declaim (inline decode-skill-sig-full))
(defun decode-skill-sig-full (key n)
  "The actual inverse function of encode-skill-sig."
  (declare (type (unsigned-byte 64) key))
  (loop
     for i below n
     for offset from 12 by 6
     collect (let ((x (logand (ash key (- offset)) #b111111)))
               (if (logbitp 5 x)
                   (dpb x (byte 6 0) -1)
                   x))))

(declaim (inline decode-skill-sig-at))
(defun decode-skill-sig-at (key i)
  "Decode the points of i-th skill from the key."
  (declare (type (unsigned-byte 64) key))
  (let* ((offset (+ 12 (* 6 i)))
	 (x (logand (ash key (- offset)) #b111111)))
    (if (logbitp 5 x)
	(dpb x (byte 6 0) -1)
	x)))


;;; ---------- Full Coding ----------
(declaim (inline encode-sig))
(defun encode-sig (hole-sig skill-sig)
  (the (unsigned-byte 64)
       (logior (encode-hole-sig hole-sig)
               (encode-skill-sig skill-sig))))

(declaim (inline decode-sig))
(defun decode-sig (key n)
  (declare (type (unsigned-byte 64) key))
  (values (decode-hole-sig key)
          (decode-skill-sig key n)))

(declaim (inline decode-sig-full))
(defun decode-sig-full (key n)
  (declare (type (unsigned-byte 64) key))
  (values (decode-hole-sig key)
          (decode-skill-sig-full key n)))

(declaim (inline hole-part))
(defun hole-part (key)
  (declare (type (unsigned-byte 64) key))
  (the (unsigned-byte 64) (ldb (byte 12 0) key)))


;;; ---------- Coding Utilities ----------

(declaim (inline encode-armor))
(defun encode-armor (armor-piece required-effects)
  (let ((hole-sig (make-list 3 :initial-element 0))
        (skill-sig (loop for (id points) in required-effects
                      collect (aif (assoc id (armor-effects armor-piece))
                                   (cadr it)
                                   0))))
    (when (> (armor-holes armor-piece) 0)
      (incf (nth (1- (armor-holes armor-piece)) hole-sig)))
    (encode-sig hole-sig skill-sig)))

;;; ---------- Coding Arithmetics ----------

(declaim (inline encoded-+))
(defun encoded-+ (key-a key-b)
  (declare (type (unsigned-byte 64) key-a))
  (declare (type (unsigned-byte 64) key-b))
  (let ((result (the (unsigned-byte 64)
                     (+ (ldb (byte 12 0) key-a)
                        (ldb (byte 12 0) key-b)))))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 12 to 57 by 6
       do (setf (ldb (byte 6 offset) result)
                (+ (ldb (byte 6 offset) key-a)
                   (ldb (byte 6 offset) key-b))))
    result))

(declaim (inline encoded-skill-+))
(defun encoded-skill-+ (key-a key-b)
  (declare (type (unsigned-byte 64) key-a))
  (declare (type (unsigned-byte 64) key-b))
  #f3
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 12 to 57 by 6
       do (setf (ldb (byte 6 offset) result)
                (+ (ldb (byte 6 offset) key-a)
                   (ldb (byte 6 offset) key-b))))
    result))


