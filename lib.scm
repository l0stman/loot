;;; Library for loot
;; Constants
(define null '())

;; Test function
(define (null? x) (eq? x null))

(define (zero? x) (= x 0))

(define (list? x)
  (or (null? x)
      (and (pair? x) (list? (cdr x)))))

(define (boolean? x)
  (or (eq? x #t) (eq? x #f)))

(define (<= x y)
  (or (= x y) (< x y)))

(define (>= x y)
  (or (= x y) (> x y)))

(define (not x)
  (if x #f #t))

(define (equal? a b)
  (cond ((number? a) (and (number? b) (= a b)))
        ((not (pair? a)) (eq? a b))
        (else (and (pair? b)
                   (equal? (car a) (car b))
                   (equal? (cdr a) (cdr b))))))

;; Search the sublist containing item in x
(define (memq item x)
  (cond ((null? x) #f)
        ((eq? item (car x)) x)
        (else (memq item (cdr x)))))

(define (assoc key alist)
  (cond ((null? alist) #f)
        ((equal? key (caar alist)) (car alist))
        (else (assoc key (cdr alist)))))

;; Access functions
(define (compose . procs)
  (define (iter res lst)
    (if (null? lst)
        res
        (iter ((car lst) res) (cdr lst))))
  (lambda (x) (iter x (reverse procs))))

(define caar (compose car car))
(define cadr (compose car cdr))
(define cdar (compose cdr car))
(define cddr (compose cdr cdr))

(define caaar (compose car car car))
(define caadr (compose car car cdr))
(define cadar (compose car cdr car))
(define caddr (compose car cdr cdr))
(define cdaar (compose cdr car car))
(define cdadr (compose cdr car cdr))
(define cddar (compose cdr cdr car))
(define cdddr (compose cdr cdr cdr))

(define caaaar (compose car car car car))
(define caaadr (compose car car car cdr))
(define caadar (compose car car cdr car))
(define caaddr (compose car car cdr cdr))
(define cadaar (compose car cdr car car))
(define cadadr (compose car cdr car cdr))
(define caddar (compose car cdr cdr car))
(define cadddr (compose car cdr cdr cdr))
(define cdaaar (compose cdr car car car))
(define cdaadr (compose cdr car car cdr))
(define cdadar (compose cdr car cdr car))
(define cdaddr (compose cdr car cdr cdr))
(define cddaar (compose cdr cdr car car))
(define cddadr (compose cdr cdr car cdr))
(define cdddar (compose cdr cdr cdr car))
(define cddddr (compose cdr cdr cdr cdr))

;; High order functions
(define (foldl f res l)
  (if (null? l)
      res
      (foldl f (f (car l) res) (cdr l))))

(define (foldr f res l) (foldl f res (reverse l)))

(define (map proc lst)
  (define (iter res lst)
    (if (null? lst)
        (reverse res)
        (iter (cons (proc (car lst)) res)
              (cdr lst))))
  (iter '() lst))

;; List functions
(define (list . l) l)

(define (length l)
  (foldl (lambda (x y) (+ 1 y)) 0 l))

(define (reverse l) (foldl cons null l))

(define (nreverse l)
  (define (loop h t)
    (if (null? h)
        t
        (let ((tmp (cdr h)))
          (set-cdr! h t)
          (loop tmp h))))
  (loop l '()))

(define (append . l)
  (define (iter h l res)
    (if (null? h)
	(if (null? l)
	    (reverse res)
	    (iter (car l) (cdr l) res))
	(iter (cdr h) l (cons (car h) res))))
  (if (null? l)
      null
      (iter (car l) (cdr l) null)))

(define (abs x)
  (if (number? x)
      (if (< x 0) (- x) x)
      (error "ABS -- not a number" x)))