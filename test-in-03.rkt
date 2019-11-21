(let*
    (
    (x 5)
    (y 6)
    (z 7)
    (a z))

    (list x y z a))

(letrec ((append1
  (lambda (L1 L2)
    (if (null? L1)
        L2
        (cons (car L1) (append1 (cdr L1) L2))))))

        (append1 (quote (1)) (quote (2))))



(define x 3)
(begin
  (set! x (+ x 1))
  (+ x x))

(begin (define x 3) (define y 4)
(+ x y))
