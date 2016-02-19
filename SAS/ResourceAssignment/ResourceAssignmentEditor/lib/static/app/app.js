// $Id$

var app = angular.module('raeApp',
                         ['DataControllerMod',
                         'GanttControllerMod',
                         'GridControllerMod']);

app.config(['$compileProvider', function ($compileProvider) {
    $compileProvider.debugInfoEnabled(false);
}]);
