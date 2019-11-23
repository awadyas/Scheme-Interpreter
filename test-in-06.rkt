;;(define weird
 ;; (lambda (x y z) 
   ;; (+ x (* y (* z z)))))
;;(weird 2 (* 6 4) 8)

;;(cond
;; ((= 2 3) (error "wrong!"))
;;((= 2 2) "ok"))

;;(define factorial
;;   (lambda (n)
;;     (if (= n 0)
;;         1
;;        (* n (factorial (- n 1))))))

;;(factorial 5)

;;(letrec ((append1
;;  (lambda (L1 L2)
;;    (if (null? L1)
;;        L2
;;        (cons (car L1) (append1 (cdr L1) L2))))))

;;(append1 (quote (1)) (quote (2))))

(define z 7)

(letrec ((x z) (y z) (m 3))
    x)

;;this example is ~funky~
(define y 5)
(letrec ((x y) (y x))
   x)
