// $Id$

var app = angular.module('raeApp',
                         ['DataControllerMod',
                         'GanttControllerMod',
                         'GridControllerMod',
                         'ui.layout']);

app.config(['$compileProvider', function ($compileProvider) {
    $compileProvider.debugInfoEnabled(false);
}]);
