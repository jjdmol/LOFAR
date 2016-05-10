(function(){
    'use strict';
    angular.module('gantt.contextmenu', ['gantt', 'gantt.contextmenu.templates']).directive('ganttContextmenu', ['$compile', '$document', function($compile, $document) {
        return {
            restrict: 'E',
            require: '^gantt',
            scope: {
                enabled: '=?'
            },
            link: function(scope, element, attrs, ganttCtrl) {
                var api = ganttCtrl.gantt.api;

                // Load options from global options attribute.
                if (scope.options && typeof(scope.options.contextmenu) === 'object') {
                    for (var option in scope.options.contextmenu) {
                        scope[option] = scope.options[option];
                    }
                }

                if (scope.enabled === undefined) {
                    scope.enabled = true;
                }

                api.directives.on.new(scope, function(dName, dScope, dElement, dAttrs, dController) {
                    //for each new ganttTask
                    if (dName === 'ganttTask') {
                        dElement.bind('contextmenu', function(event) {
                            //TODO: remove link to dataService in this generic plugin
                            var dataService = dScope.scope.dataService;

                            if(dScope.task.model.raTask) {
                                dataService.selected_task_id = dScope.task.model.raTask.id;
                            }

                            //search for already existing contextmenu element
                            if(dElement.find('#gantt-context-menu').length) {
                                //found, remove it, so we can create a fresh one
                                dElement.find('#gantt-context-menu')[0].remove();
                            }

                            //create contextmenu element
                            //with list of menu items,
                            //each with it's own action
                            var contextmenuElement = angular.element('<div id="gantt-context-menu"></div>');
                            var ulElement = angular.element('<ul style="z-index:10000; position:fixed; top:initial; left:initial; display:block;" role="menu" class="dropdown-menu"></ul>');
                            contextmenuElement.append(ulElement);
                            var liElement = angular.element('<li><a href="#">Copy Task</a></li>');
                            ulElement.append(liElement);
                            liElement.on('click', function() {
                                //TODO: remove link to dataService in this generic plugin
                                dataService.copyTask(dScope.task.model.raTask);
                                closeContextMenu();
                            });

                            var closeContextMenu = function() {
                                contextmenuElement.remove();
                                angular.element(document).unbind('click', closeContextMenu);
                            };

                            //click anywhere to remove the contextmenu
                            angular.element(document).bind('click', closeContextMenu);

                            //add contextmenu to clicked element
                            dElement.append(contextmenuElement);

                            //prevent bubbling event upwards
                            return false;
                        });
                    }
                });

                api.directives.on.destroy(scope, function(dName, dScope, dElement, dAttrs, dController) {
                    //for each destroyed ganttTask
                    if (dName === 'ganttTask') {
                        dElement.unbind('contextmenu');
                    }
                });
            }
        };
    }]);
}());

angular.module('gantt.contextmenu.templates', []).run(['$templateCache', function($templateCache) {

}]);

