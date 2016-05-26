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

app.filter('secondsToHHmmss', function($filter) {
    return function(seconds) {
        return $filter('date')(new Date(0, 0, 0).setSeconds(seconds), 'HH:mm:ss');
    };
})
