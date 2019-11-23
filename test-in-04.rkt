(define A
  (lambda (k x1 x2 x3 x4 x5)
    (letrec ((B
              (lambda ()
                (begin
                  (set! k (- k 1))
                  (A k B x1 x2 x3 x4)))))
      (if (or (< k 0) (= k 0))
          (+ (x4) (x5))
          (B)))))

(A 10 (lambda () 1) (lambda () -1) (lambda () -1) (lambda () 1) (lambda () 0))

;;(load "test-in-01.rkt")
