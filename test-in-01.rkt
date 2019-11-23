(define fun
    (lambda ()
        7))
(fun)

(cons ((lambda () 1)) ((lambda () 2)))

((lambda () 1))

(define fun2
    (lambda (x y)
        (+ (x) (y))))

(fun2 (lambda () 1) (lambda () 2))

((lambda (x y ) (+ x y)) 3 4)
((lambda () (+ 3 4)))

(cons (lambda () 1) (lambda () 2))


