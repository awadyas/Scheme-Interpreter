(define append1
  (lambda (L1 L2)
    (if (null? L1)
        L2
        (cons (car L1) (append1 (cdr L1) L2)))))

(quote (4 5 (6)))
(quote (6 7))
(append1 (quote (4 5)) (quote (6 7)))

(cons 3 4)
(cons 6.66 "hi")
(cons 4 (quote (9 0 0)))


(car (quote (1 2 3)))

(cdr (quote (1 2 3)))

(define reverse-list
  (lambda (L)
    (if (null? L)
        L
        (append1 (reverse-list (cdr L)) (cons (car L) (quote ()))))))

(reverse-list (quote ()))
(reverse-list (quote (1 2 3 4)))
(reverse-list (quote (("computer" "science") "is" "not" "awesome")))

;;(cdr (quote (1 2)) (quote (2 3)))


(null? (quote (1 3 5 2)))
(null? "hello")
(define x 1)
(null? x)
(null? (quote ()))
(+ 1 2)
(+ 1 2 3)
(+)
(+ 4.44 5 -9.7)
;;(+ 1 "hello")

(equal? 3 3)
(equal? x x)
(equal? "my oh my" "my oh my")
(equal? 3 "hello")

(list (quote(1 2)) (quote(3 4)))
(list (quote(1 2)) (quote(3 4)) x)

(append (quote(1 2)) (quote(3 4)))
(append (quote ((1 2) (3 4))) (quote ((5 6) (7 8))))
(append (quote(1 2)) x (quote(3 4)))
