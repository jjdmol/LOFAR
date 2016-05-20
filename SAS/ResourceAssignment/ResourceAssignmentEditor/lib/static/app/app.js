// $Id$

var app = angular.module('raeApp',
                         ['DataControllerMod',
                         'GanttResourceControllerMod',
                         'GanttProjectControllerMod',
                         'ChartResourceUsageControllerMod',
                         'GridControllerMod',
                         'ui.layout',
                         'ui.bootstrap',
                         'ui.bootstrap.tabs',
                         'highcharts-ng']);

app.config(['$compileProvider', function ($compileProvider) {
    $compileProvider.debugInfoEnabled(false);
}]);
