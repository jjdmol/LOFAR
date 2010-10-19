from django.conf.urls.defaults import *
import playpen.data as data

urlpatterns = patterns('',
                       # Example:
                       #(r'^mysite/', include('mysite.foo.urls')),
                       
                       # Uncomment this for admin:
                       #(r'^admin/', include('django.contrib.admin.urls')),
                       (r'(?P<path>.*css)$','django.views.static.serve', {'document_root': '/home/avruch/work/lofar/shm/versions/Components/WebInterface/testing/djstart/playpen'}),
                       (r'(?P<path>.*png)$','django.views.static.serve', {'document_root': '/home/avruch/work/lofar/shm/versions/Components/WebInterface/testing/djstart/playpen'}),
                       (r'^shm/$',data.defaultpage),
                       (r'^shm/data/$', data.defaultpage),
                       (r'^shm/data/xst/(?P<station>[cr]s.*)/image/thumb/(?P<plotnumber>\d+)/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>[cr]s.*)/image/(?P<plotnumber>\d+)/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>[cr]s.*)/image/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>[cr]s.*)/fetch/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>[cr]s.*)/$', 'data.xst'),
                       (r'^shm/data/sky/(?P<station>[cr]s.*)/image/thumb/(?P<plotnumber>\d+)/$',data.sky),
                       (r'^shm/data/sky/(?P<station>[cr]s.*)/image/(?P<plotnumber>\d+)/$',data.sky),
                       (r'^shm/data/sky/(?P<station>[cr]s.*)/image/$',data.sky),
                       (r'^shm/data/sky/(?P<station>[cr]s.*)/fetch/$',data.sky),
                       (r'^shm/data/sky/(?P<station>[cr]s.*)/$', data.sky),
                       (r'^shm/data/sst/(?P<station>[cr]s.*)/image/thumb/(?P<plotnumber>\d+)/$',data.sst),
                       (r'^shm/data/sst/(?P<station>[cr]s.*)/image/(?P<plotnumber>\d+)/$',data.sst),
                       (r'^shm/data/sst/(?P<station>[cr]s.*)/image/$',data.sst),
                       (r'^shm/data/sst/(?P<station>[cr]s.*)/fetch/$','data.sst'),
                       (r'^shm/data/sst/(?P<station>[cr]s.*)/$', 'data.sst'),
                       (r'^shm/data/rsp/(?P<station>[cr]s.*)/image/(?P<plotnumber>\d+)/$','data.rsp'),
                       (r'^shm/data/rsp/(?P<station>[cr]s.*)/image/$','data.rsp'),
                       (r'^shm/data/rsp/(?P<station>[cr]s.*)/fetch/$','data.rsp'),
                       (r'^shm/data/rsp/(?P<station>[cr]s.*)/$', 'data.rsp'),
                       )
