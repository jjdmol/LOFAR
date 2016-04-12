// $Id$

var app = angular.module('raeApp',
                         ['DataControllerMod',
                         'GanttResourceControllerMod',
                         'GanttProjectControllerMod',
                         'ChartResourceUsageControllerMod',
                         'GridControllerMod',
                         'ui.layout',
                         'ui.bootstrap.datetimepicker',
                         'ui.bootstrap.tabs',
                         'highcharts-ng']);

app.config(['$compileProvider', function ($compileProvider) {
    $compileProvider.debugInfoEnabled(false);
}]);
