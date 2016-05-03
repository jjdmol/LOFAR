--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: exchangeroutes; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE exchangeroutes (
    erouteid integer NOT NULL,
    fromhost bigint NOT NULL,
    tohost bigint NOT NULL,
    eid bigint NOT NULL,
    routingkey character varying(512) NOT NULL
);


ALTER TABLE public.exchangeroutes OWNER TO peterzon;

--
-- Name: exchangeroutes_erouteid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE exchangeroutes_erouteid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.exchangeroutes_erouteid_seq OWNER TO peterzon;

--
-- Name: exchangeroutes_erouteid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE exchangeroutes_erouteid_seq OWNED BY exchangeroutes.erouteid;


--
-- Name: exchanges; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE exchanges (
    exchangeid integer NOT NULL,
    exchangename character varying(512) NOT NULL
);


ALTER TABLE public.exchanges OWNER TO peterzon;

--
-- Name: exchanges_exchangeid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE exchanges_exchangeid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.exchanges_exchangeid_seq OWNER TO peterzon;

--
-- Name: exchanges_exchangeid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE exchanges_exchangeid_seq OWNED BY exchanges.exchangeid;


--
-- Name: hosts; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE hosts (
    hostid integer NOT NULL,
    hostname character varying(512) NOT NULL
);


ALTER TABLE public.hosts OWNER TO peterzon;

--
-- Name: hosts_hostid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE hosts_hostid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.hosts_hostid_seq OWNER TO peterzon;

--
-- Name: hosts_hostid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE hosts_hostid_seq OWNED BY hosts.hostid;


--
-- Name: persistentexchanges; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE persistentexchanges (
    pexid integer NOT NULL,
    eid bigint NOT NULL,
    hid bigint NOT NULL
);


ALTER TABLE public.persistentexchanges OWNER TO peterzon;

--
-- Name: persistentexchanges_pexid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE persistentexchanges_pexid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.persistentexchanges_pexid_seq OWNER TO peterzon;

--
-- Name: persistentexchanges_pexid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE persistentexchanges_pexid_seq OWNED BY persistentexchanges.pexid;


--
-- Name: persistentqueues; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE persistentqueues (
    pquid integer NOT NULL,
    qid bigint NOT NULL,
    hid bigint NOT NULL
);


ALTER TABLE public.persistentqueues OWNER TO peterzon;

--
-- Name: persistentqueues_pquid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE persistentqueues_pquid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.persistentqueues_pquid_seq OWNER TO peterzon;

--
-- Name: persistentqueues_pquid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE persistentqueues_pquid_seq OWNED BY persistentqueues.pquid;


--
-- Name: queuelistener; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE queuelistener (
    qlistenid integer NOT NULL,
    fromhost bigint NOT NULL,
    eid bigint NOT NULL,
    qid bigint NOT NULL,
    subject character varying(512) NOT NULL
);


ALTER TABLE public.queuelistener OWNER TO peterzon;

--
-- Name: queuelistener_qlistenid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE queuelistener_qlistenid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.queuelistener_qlistenid_seq OWNER TO peterzon;

--
-- Name: queuelistener_qlistenid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE queuelistener_qlistenid_seq OWNED BY queuelistener.qlistenid;


--
-- Name: queueroutes; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE queueroutes (
    qrouteid integer NOT NULL,
    fromhost bigint NOT NULL,
    tohost bigint NOT NULL,
    qid bigint NOT NULL
);


ALTER TABLE public.queueroutes OWNER TO peterzon;

--
-- Name: queueroutes_qrouteid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE queueroutes_qrouteid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.queueroutes_qrouteid_seq OWNER TO peterzon;

--
-- Name: queueroutes_qrouteid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE queueroutes_qrouteid_seq OWNED BY queueroutes.qrouteid;


--
-- Name: queues; Type: TABLE; Schema: public; Owner: peterzon; Tablespace: 
--

CREATE TABLE queues (
    queueid integer NOT NULL,
    queuename character varying(512) NOT NULL
);


ALTER TABLE public.queues OWNER TO peterzon;

--
-- Name: queues_queueid_seq; Type: SEQUENCE; Schema: public; Owner: peterzon
--

CREATE SEQUENCE queues_queueid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.queues_queueid_seq OWNER TO peterzon;

--
-- Name: queues_queueid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: peterzon
--

ALTER SEQUENCE queues_queueid_seq OWNED BY queues.queueid;


--
-- Name: erouteid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY exchangeroutes ALTER COLUMN erouteid SET DEFAULT nextval('exchangeroutes_erouteid_seq'::regclass);


--
-- Name: exchangeid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY exchanges ALTER COLUMN exchangeid SET DEFAULT nextval('exchanges_exchangeid_seq'::regclass);


--
-- Name: hostid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY hosts ALTER COLUMN hostid SET DEFAULT nextval('hosts_hostid_seq'::regclass);


--
-- Name: pexid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY persistentexchanges ALTER COLUMN pexid SET DEFAULT nextval('persistentexchanges_pexid_seq'::regclass);


--
-- Name: pquid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY persistentqueues ALTER COLUMN pquid SET DEFAULT nextval('persistentqueues_pquid_seq'::regclass);


--
-- Name: qlistenid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY queuelistener ALTER COLUMN qlistenid SET DEFAULT nextval('queuelistener_qlistenid_seq'::regclass);


--
-- Name: qrouteid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY queueroutes ALTER COLUMN qrouteid SET DEFAULT nextval('queueroutes_qrouteid_seq'::regclass);


--
-- Name: queueid; Type: DEFAULT; Schema: public; Owner: peterzon
--

ALTER TABLE ONLY queues ALTER COLUMN queueid SET DEFAULT nextval('queues_queueid_seq'::regclass);


--
-- Data for Name: exchangeroutes; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY exchangeroutes (erouteid, fromhost, tohost, eid, routingkey) FROM stdin;
\.


--
-- Name: exchangeroutes_erouteid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('exchangeroutes_erouteid_seq', 1, false);


--
-- Data for Name: exchanges; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY exchanges (exchangeid, exchangename) FROM stdin;
1	lofar.ra.command
2	lofar.ra.notification
3	lofar.otdb.command
4	lofar.otdb.notification
5	lofar.sm.command
6	lofar.sm.notification
7	lofar.mom.command
8	lofar.mom.notification
\.


--
-- Name: exchanges_exchangeid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('exchanges_exchangeid_seq', 8, true);


--
-- Data for Name: hosts; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY hosts (hostid, hostname) FROM stdin;
1	scu001.control.lofar
2	ccu001.control.lofar
3	head01.control.lofar
4	lhn001.cep2.lofar
5	mcu001.control.lofar
6	lcs023.control.lofar
7	sas001.control.lofar
8	locus001.cep2.lofar
9	locus002.cep2.lofar
10	locus003.cep2.lofar
11	locus004.cep2.lofar
12	locus005.cep2.lofar
13	locus006.cep2.lofar
14	locus007.cep2.lofar
15	locus008.cep2.lofar
16	locus009.cep2.lofar
17	locus010.cep2.lofar
18	locus011.cep2.lofar
19	locus012.cep2.lofar
20	locus013.cep2.lofar
21	locus014.cep2.lofar
22	locus015.cep2.lofar
23	locus016.cep2.lofar
24	locus017.cep2.lofar
25	locus018.cep2.lofar
26	locus019.cep2.lofar
27	locus020.cep2.lofar
28	locus021.cep2.lofar
29	locus022.cep2.lofar
30	locus023.cep2.lofar
31	locus024.cep2.lofar
32	locus025.cep2.lofar
33	locus026.cep2.lofar
34	locus027.cep2.lofar
35	locus028.cep2.lofar
36	locus029.cep2.lofar
37	locus030.cep2.lofar
38	locus031.cep2.lofar
39	locus032.cep2.lofar
40	locus033.cep2.lofar
41	locus034.cep2.lofar
42	locus035.cep2.lofar
43	locus036.cep2.lofar
44	locus037.cep2.lofar
45	locus038.cep2.lofar
46	locus039.cep2.lofar
47	locus040.cep2.lofar
48	locus041.cep2.lofar
49	locus042.cep2.lofar
50	locus043.cep2.lofar
51	locus044.cep2.lofar
52	locus045.cep2.lofar
53	locus046.cep2.lofar
54	locus047.cep2.lofar
55	locus048.cep2.lofar
56	locus049.cep2.lofar
57	locus050.cep2.lofar
58	locus051.cep2.lofar
59	locus052.cep2.lofar
60	locus053.cep2.lofar
61	locus054.cep2.lofar
62	locus055.cep2.lofar
63	locus056.cep2.lofar
64	locus057.cep2.lofar
65	locus058.cep2.lofar
66	locus059.cep2.lofar
67	locus060.cep2.lofar
68	locus061.cep2.lofar
69	locus062.cep2.lofar
70	locus063.cep2.lofar
71	locus064.cep2.lofar
72	locus065.cep2.lofar
73	locus066.cep2.lofar
74	locus067.cep2.lofar
75	locus068.cep2.lofar
76	locus069.cep2.lofar
77	locus070.cep2.lofar
78	locus071.cep2.lofar
79	locus072.cep2.lofar
80	locus073.cep2.lofar
81	locus074.cep2.lofar
82	locus075.cep2.lofar
83	locus076.cep2.lofar
84	locus077.cep2.lofar
85	locus078.cep2.lofar
86	locus079.cep2.lofar
87	locus080.cep2.lofar
88	locus081.cep2.lofar
89	locus082.cep2.lofar
90	locus083.cep2.lofar
91	locus084.cep2.lofar
92	locus085.cep2.lofar
93	locus086.cep2.lofar
94	locus087.cep2.lofar
95	locus088.cep2.lofar
96	locus089.cep2.lofar
97	locus090.cep2.lofar
98	locus091.cep2.lofar
99	locus092.cep2.lofar
100	locus093.cep2.lofar
101	locus094.cep2.lofar
\.


--
-- Name: hosts_hostid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('hosts_hostid_seq', 101, true);


--
-- Data for Name: persistentexchanges; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY persistentexchanges (pexid, eid, hid) FROM stdin;
1	1	1
2	2	1
3	3	1
4	4	1
5	5	1
6	6	1
7	7	1
8	8	1
\.


--
-- Name: persistentexchanges_pexid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('persistentexchanges_pexid_seq', 8, true);


--
-- Data for Name: persistentqueues; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY persistentqueues (pquid, qid, hid) FROM stdin;
1	1	1
2	2	1
3	3	1
4	4	4
5	5	4
6	6	4
7	4	2
8	5	2
9	6	2
10	7	2
11	8	2
12	9	2
13	10	2
14	11	2
15	12	2
16	13	2
17	7	5
18	12	5
19	13	5
20	14	6
21	15	6
22	16	6
23	8	6
24	9	6
25	10	6
26	11	6
27	14	7
28	15	7
29	16	7
30	4	8
31	5	8
32	6	8
33	4	9
34	5	9
35	6	9
36	4	10
37	5	10
38	6	10
39	4	11
40	5	11
41	6	11
42	4	12
43	5	12
44	6	12
45	4	13
46	5	13
47	6	13
48	4	14
49	5	14
50	6	14
51	4	15
52	5	15
53	6	15
54	4	16
55	5	16
56	6	16
57	4	17
58	5	17
59	6	17
60	4	18
61	5	18
62	6	18
63	4	19
64	5	19
65	6	19
66	4	20
67	5	20
68	6	20
69	4	21
70	5	21
71	6	21
72	4	22
73	5	22
74	6	22
75	4	23
76	5	23
77	6	23
78	4	24
79	5	24
80	6	24
81	4	25
82	5	25
83	6	25
84	4	26
85	5	26
86	6	26
87	4	27
88	5	27
89	6	27
90	4	28
91	5	28
92	6	28
93	4	29
94	5	29
95	6	29
96	4	30
97	5	30
98	6	30
99	4	31
100	5	31
101	6	31
102	4	32
103	5	32
104	6	32
105	4	33
106	5	33
107	6	33
108	4	34
109	5	34
110	6	34
111	4	35
112	5	35
113	6	35
114	4	36
115	5	36
116	6	36
117	4	37
118	5	37
119	6	37
120	4	38
121	5	38
122	6	38
123	4	39
124	5	39
125	6	39
126	4	40
127	5	40
128	6	40
129	4	41
130	5	41
131	6	41
132	4	42
133	5	42
134	6	42
135	4	43
136	5	43
137	6	43
138	4	44
139	5	44
140	6	44
141	4	45
142	5	45
143	6	45
144	4	46
145	5	46
146	6	46
147	4	47
148	5	47
149	6	47
150	4	48
151	5	48
152	6	48
153	4	49
154	5	49
155	6	49
156	4	50
157	5	50
158	6	50
159	4	51
160	5	51
161	6	51
162	4	52
163	5	52
164	6	52
165	4	53
166	5	53
167	6	53
168	4	54
169	5	54
170	6	54
171	4	55
172	5	55
173	6	55
174	4	56
175	5	56
176	6	56
177	4	57
178	5	57
179	6	57
180	4	58
181	5	58
182	6	58
183	4	59
184	5	59
185	6	59
186	4	60
187	5	60
188	6	60
189	4	61
190	5	61
191	6	61
192	4	62
193	5	62
194	6	62
195	4	63
196	5	63
197	6	63
198	4	64
199	5	64
200	6	64
201	4	65
202	5	65
203	6	65
204	4	66
205	5	66
206	6	66
207	4	67
208	5	67
209	6	67
210	4	68
211	5	68
212	6	68
213	4	69
214	5	69
215	6	69
216	4	70
217	5	70
218	6	70
219	4	71
220	5	71
221	6	71
222	4	72
223	5	72
224	6	72
225	4	73
226	5	73
227	6	73
228	4	74
229	5	74
230	6	74
231	4	75
232	5	75
233	6	75
234	4	76
235	5	76
236	6	76
237	4	77
238	5	77
239	6	77
240	4	78
241	5	78
242	6	78
243	4	79
244	5	79
245	6	79
246	4	80
247	5	80
248	6	80
249	4	81
250	5	81
251	6	81
252	4	82
253	5	82
254	6	82
255	4	83
256	5	83
257	6	83
258	4	84
259	5	84
260	6	84
261	4	85
262	5	85
263	6	85
264	4	86
265	5	86
266	6	86
267	4	87
268	5	87
269	6	87
270	4	88
271	5	88
272	6	88
273	4	89
274	5	89
275	6	89
276	4	90
277	5	90
278	6	90
279	4	91
280	5	91
281	6	91
282	4	92
283	5	92
284	6	92
285	4	93
286	5	93
287	6	93
288	4	94
289	5	94
290	6	94
291	4	95
292	5	95
293	6	95
294	4	96
295	5	96
296	6	96
297	4	97
298	5	97
299	6	97
300	4	98
301	5	98
302	6	98
303	4	99
304	5	99
305	6	99
306	4	100
307	5	100
308	6	100
309	4	101
310	5	101
311	6	101
\.


--
-- Name: persistentqueues_pquid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('persistentqueues_pquid_seq', 311, true);


--
-- Data for Name: queuelistener; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY queuelistener (qlistenid, fromhost, eid, qid, subject) FROM stdin;
\.


--
-- Name: queuelistener_qlistenid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('queuelistener_qlistenid_seq', 1, false);


--
-- Data for Name: queueroutes; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY queueroutes (qrouteid, fromhost, tohost, qid) FROM stdin;
\.


--
-- Name: queueroutes_qrouteid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('queueroutes_qrouteid_seq', 1, false);


--
-- Data for Name: queues; Type: TABLE DATA; Schema: public; Owner: peterzon
--

COPY queues (queueid, queuename) FROM stdin;
1	TreeStatus
2	TaskSpecified
3	ResourceAssigner
4	lofar.task.feedback.dataproducts
5	lofar.task.feedback.processing
6	lofar.task.feedback.state
7	lofar.task.specification.system
8	mom.task.feedback.dataproducts
9	mom.task.feedback.processing
10	mom.task.feedback.state
11	mom.task.specification.system
12	otdb.task.feedback.dataproducts
13	otdb.task.feedback.processing
14	mom-otdb-adapter.importxml
15	mom.command
16	mom.importxml
\.


--
-- Name: queues_queueid_seq; Type: SEQUENCE SET; Schema: public; Owner: peterzon
--

SELECT pg_catalog.setval('queues_queueid_seq', 16, true);


--
-- Name: exchangeroutes_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY exchangeroutes
    ADD CONSTRAINT exchangeroutes_pkey PRIMARY KEY (erouteid);


--
-- Name: exchanges_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY exchanges
    ADD CONSTRAINT exchanges_pkey PRIMARY KEY (exchangeid);


--
-- Name: hosts_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY hosts
    ADD CONSTRAINT hosts_pkey PRIMARY KEY (hostid);


--
-- Name: persistentexchanges_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY persistentexchanges
    ADD CONSTRAINT persistentexchanges_pkey PRIMARY KEY (pexid);


--
-- Name: persistentqueues_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY persistentqueues
    ADD CONSTRAINT persistentqueues_pkey PRIMARY KEY (pquid);


--
-- Name: queuelistener_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY queuelistener
    ADD CONSTRAINT queuelistener_pkey PRIMARY KEY (qlistenid);


--
-- Name: queueroutes_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY queueroutes
    ADD CONSTRAINT queueroutes_pkey PRIMARY KEY (qrouteid);


--
-- Name: queues_pkey; Type: CONSTRAINT; Schema: public; Owner: peterzon; Tablespace: 
--

ALTER TABLE ONLY queues
    ADD CONSTRAINT queues_pkey PRIMARY KEY (queueid);


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

