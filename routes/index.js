var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res) {
  // logger.statsd.increment('demos.cae_report');
  res.render('cae-report', { title: 'CAE Analysis Report | HOOPS Web Viewer'});
});

module.exports = router;
