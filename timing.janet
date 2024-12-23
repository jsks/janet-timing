(import timing/native)

(defmacro- thunk [& body]
  ~(fn [] ,;body))

(defn- time-thunk [fun times warmup]
  (let [[mean iqr-low iqr-high stddev] (native/nstime-function fun (fn []) times warmup)]
    {:mean mean
     :stddev stddev
     :iqr-low iqr-low
     :iqr-high iqr-high}))

(defn- convert [result divisor]
  (tabseq [[k v] :pairs result] k (/ v divisor)))

(defn- result-string [{:mean mean :stddev stddev :iqr-low iqr-low :iqr-high iqr-high} unit]
  (if (= math/inf stddev)
    (string/format "%.2f %s" mean unit)
    (string/format "%.2f %s [± %.2f %s] IQR: %.2f .. %.2f"
                   mean unit stddev unit iqr-low iqr-high)))

(defn- process-result [result]
  (let [mean (result :mean)]
    (cond
      (< mean 1e3) (result-string result "ns")
      (< mean 1e6) (result-string (convert result 1e3) "us")
      (< mean 1e9) (result-string (convert result 1e6) "ms")
      (< mean 6e10) (result-string (convert result 1e9) "s")
      (result-string (convert result 6e10) "min"))))

(defmacro elapsed
  ``Macro that evaluates `form` and prints the execution time.

    When repeated evaluations are run using the `:times` argument,
    this macro prints the mean, standard deviation, and inter-quartile
    range of the samples.

    Optional arguments:
    - :times -- Number of times to evaluate `form`
    - :warmup -- Number of warmup evaluations of `form` to run before
                 measuring execution time
  ``
  [form &named times warmup]
  (default times '1)
  (default warmup '1)
  ~(let [result (time-thunk (thunk ,form) ,times ,warmup)]
     (when (neg? (result :mean))
       (eprint "Warning: overhead execution greater than benchmark. Try increasing :times argument."))
     (print (process-result result))))
